#include <Adore/Internal/Vulkan/Buffer.hpp>
#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Log.hpp>

#include <stb_image.h>

VkFilter getVulkanFilter(Adore::Filter const& filter)
{
    switch (filter)
    {
        case Adore::Filter::LINEAR: return VK_FILTER_LINEAR;
        case Adore::Filter::NEAREST: return VK_FILTER_NEAREST;
    }
}

VkSamplerAddressMode getVulkanWrap(Adore::Wrap const& wrap)
{
    switch (wrap)
    {
        case Adore::Wrap::REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case Adore::Wrap::MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case Adore::Wrap::CLAMP_TO_EDGE: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case Adore::Wrap::CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
}

uint32_t memoryTypeIndex(uint32_t const& memoryTypeBits,
                         VkPhysicalDevice const& physicalDevice,
                         VkMemoryPropertyFlags const& properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (memoryTypeBits & (1 << i)
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw Adore::AdoreException("Failed to find suitable Vulkan memory type.");
}

void createBuffer(VkDevice const& device, VkPhysicalDevice const& physicalDevice,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan buffer.");

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex(memReqs.memoryTypeBits, physicalDevice, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to allocate Vulkan buffer memory.");

    if (vkBindBufferMemory(device, buffer, bufferMemory, 0) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to bind Vulkan buffer memory.");
}

void copyBuffer(VulkanRenderer * prenderer, VkBuffer const& srcBuffer, VkBuffer const& dstBuffer, VkDeviceSize size)
{
    auto cmdBuf = prenderer->beginCommandBuffer();

    VkBufferCopy copyRegion {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(cmdBuf, srcBuffer, dstBuffer, 1, &copyRegion);

    prenderer->endCommandBuffer(cmdBuf);
}

void transitionImageLayout(VulkanRenderer * prenderer, VkImage const& image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = prenderer->beginCommandBuffer();

    VkImageMemoryBarrier barrier {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkPipelineStageFlags srcStage, dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT; 
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier
    (
        commandBuffer,
        srcStage, dstStage,
        0, 0, nullptr,
        0, nullptr,
        1, &barrier
    );

    prenderer->endCommandBuffer(commandBuffer);
}

void copyBufferToImage(VulkanRenderer * prenderer, VkBuffer const& buffer, VkImage const& image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = prenderer->beginCommandBuffer();

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage
    (
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    prenderer->endCommandBuffer(commandBuffer);
}

VulkanIndexBuffer::VulkanIndexBuffer(std::shared_ptr<Adore::Renderer>& renderer,
                        void* pdata, uint64_t const& size)
    : Adore::IndexBuffer(renderer)
{
    VulkanRenderer * prenderer = static_cast<VulkanRenderer*>(m_renderer.get());
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(pwindow->device(), pwindow->physicalDevice(), size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void * map;
    vkMapMemory(pwindow->device(), stagingBufferMemory, 0, size, 0, &map);
        memcpy(map, pdata, size);
    vkUnmapMemory(pwindow->device(), stagingBufferMemory);

    createBuffer(pwindow->device(), pwindow->physicalDevice(), size,
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_buffer, m_memory);

    copyBuffer(prenderer, stagingBuffer, m_buffer, size);

    vkDestroyBuffer(pwindow->device(), stagingBuffer, nullptr);
    vkFreeMemory(pwindow->device(), stagingBufferMemory, nullptr);

    ADORE_INTERNAL_LOG(INFO, "Created Vulkan Index buffer.");
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());
    vkDestroyBuffer(pwindow->device(), m_buffer, nullptr);
    vkFreeMemory(pwindow->device(), m_memory, nullptr);
}

VulkanVertexBuffer::VulkanVertexBuffer(std::shared_ptr<Adore::Renderer>& renderer,
                        void* pdata, uint64_t const& size)
    : Adore::VertexBuffer(renderer)
{
    VulkanRenderer * prenderer = static_cast<VulkanRenderer*>(m_renderer.get());
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(pwindow->device(), pwindow->physicalDevice(), size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void * map;
    vkMapMemory(pwindow->device(), stagingBufferMemory, 0, size, 0, &map);
        memcpy(map, pdata, size);
    vkUnmapMemory(pwindow->device(), stagingBufferMemory);

    createBuffer(pwindow->device(), pwindow->physicalDevice(), size,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 m_buffer, m_memory);

    auto cmdBuf = prenderer->beginCommandBuffer();

    VkBufferCopy copyRegion {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(cmdBuf, stagingBuffer, m_buffer, 1, &copyRegion);

    prenderer->endCommandBuffer(cmdBuf);

    vkDestroyBuffer(pwindow->device(), stagingBuffer, nullptr);
    vkFreeMemory(pwindow->device(), stagingBufferMemory, nullptr);

    ADORE_INTERNAL_LOG(INFO, "Created Vulkan Vertex buffer.");
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());
    vkDestroyBuffer(pwindow->device(), m_buffer, nullptr);
    vkFreeMemory(pwindow->device(), m_memory, nullptr);
}

VulkanUniformBuffer::VulkanUniformBuffer(std::shared_ptr<Adore::Renderer>& renderer,
                        void* pdata, uint64_t const& size)
    : Adore::UniformBuffer(renderer, pdata, size)
{
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());

    m_changed.resize(FRAMES_IN_FLIGHT, false);
    m_buffers.resize(FRAMES_IN_FLIGHT);
    m_memories.resize(FRAMES_IN_FLIGHT);
    m_maps.resize(FRAMES_IN_FLIGHT);

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
        createBuffer(pwindow->device(), pwindow->physicalDevice(), size,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_buffers[i], m_memories[i]);
        vkMapMemory(pwindow->device(), m_memories[i], 0, size, 0, &m_maps[i]);
        memcpy(m_maps[i], pdata, size);
    }

}

void VulkanUniformBuffer::set(void const * pdata)
{
    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++)
        m_changed[i] = true;
    memcpy(m_pUniformObject, pdata, m_size);
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());
    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(pwindow->device(), m_buffers[i], nullptr);
        vkFreeMemory(pwindow->device(), m_memories[i], nullptr);
    }
}

void VulkanUniformBuffer::update(size_t const& index)
{
    if (m_changed[index])
    {
        memcpy(m_maps[index], m_pUniformObject, m_size);
        m_changed[index] = false;
    }
}

VkBuffer VulkanUniformBuffer::buffer(size_t const& index)
{
    return m_buffers[index];
}

VulkanSampler::VulkanSampler(std::shared_ptr<Adore::Renderer>& renderer, const char* path,
                             Adore::Filter const& filter, Adore::Wrap const& wrap)
    : Adore::Sampler(renderer)
{
    VulkanRenderer * prenderer = static_cast<VulkanRenderer*>(m_renderer.get());
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());

    int width, height, channels;
    stbi_uc * pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) throw Adore::AdoreException("Failed to load image: " + std::string(path));

    VkDeviceSize imageSize = width * height * 4; // 1 byte per channel

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(pwindow->device(), pwindow->physicalDevice(), imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(pwindow->device(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, imageSize);
    vkUnmapMemory(pwindow->device(), stagingBufferMemory);

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(pwindow->device(), &imageInfo, nullptr, &m_image) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan image.");

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(pwindow->device(), m_image, &memReqs);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex(memReqs.memoryTypeBits, 
                                                pwindow->physicalDevice(),
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                                
    if (vkAllocateMemory(pwindow->device(), &allocInfo, nullptr, &m_memory) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to allocate Vulkan image memory.");
    
    vkBindImageMemory(pwindow->device(), m_image, m_memory, 0);

    transitionImageLayout(prenderer, m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    copyBufferToImage(prenderer, stagingBuffer, m_image, width, height);

    vkDestroyBuffer(pwindow->device(), stagingBuffer, nullptr);
    vkFreeMemory(pwindow->device(), stagingBufferMemory, nullptr);

    transitionImageLayout(prenderer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    if (vkCreateImageView(pwindow->device(), &viewInfo, nullptr, &m_view) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan image view.");

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = getVulkanFilter(filter);
    samplerInfo.minFilter = getVulkanFilter(filter);
    samplerInfo.addressModeU = getVulkanWrap(wrap);
    samplerInfo.addressModeV = getVulkanWrap(wrap);
    samplerInfo.addressModeW = getVulkanWrap(wrap);
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(pwindow->physicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(pwindow->device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan sampler.");
}

VulkanSampler::~VulkanSampler()
{
    VulkanWindow * pwindow = static_cast<VulkanWindow*>(m_renderer->window().get());
    vkDestroySampler(pwindow->device(), m_sampler, nullptr);
    vkDestroyImageView(pwindow->device(), m_view, nullptr);
    vkDestroyImage(pwindow->device(), m_image, nullptr);
    vkFreeMemory(pwindow->device(), m_memory, nullptr);
}