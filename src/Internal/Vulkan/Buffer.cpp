#include <Adore/Internal/Vulkan/Buffer.hpp>
#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Log.hpp>

void createBuffer(VkDevice const& device, VkPhysicalDevice const& physicalDevice, VkDeviceSize size,
                  VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
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

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (memReqs.memoryTypeBits & (1 << i)
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            VkMemoryAllocateInfo allocInfo {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memReqs.size;
            allocInfo.memoryTypeIndex = i;

            if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
                throw Adore::AdoreException("Failed to allocate Vulkan buffer memory.");

            if (vkBindBufferMemory(device, buffer, bufferMemory, 0) != VK_SUCCESS)
                throw Adore::AdoreException("Failed to bind Vulkan buffer memory.");
            
            return;
        }
    }

    throw Adore::AdoreException("Failed to find suitable Vulkan buffer memory.");
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

    prenderer->copy(stagingBuffer, m_buffer, size);

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

    prenderer->copy(stagingBuffer, m_buffer, size);

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