#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Log.hpp>

#include <map>
#include <set>
#include <algorithm>

Swapchain::Swapchain(VkDevice const& device, VkSurfaceKHR const& surface, VkSurfaceFormatKHR const& format,
                     VkPresentModeKHR const& mode, uint32_t imageCount, VkExtent2D const& extent,
                     std::vector<uint32_t> const& queueIndices, VkRenderPass const& renderPass)
    : m_device(device)
{
    VkSwapchainCreateInfoKHR swapchainInfo {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = surface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = format.format;
    swapchainInfo.imageColorSpace = format.colorSpace;
    swapchainInfo.imageExtent = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (std::equal(queueIndices.begin() + 1, queueIndices.begin(), queueIndices.end()))
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = queueIndices.size();
        swapchainInfo.pQueueFamilyIndices = queueIndices.data();
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; //.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = mode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create a Vulkan swapchain.");

    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, m_images.data());

    m_imageViews.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format.format;
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };

        if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create a Vulkan image view.");
    }

    m_framebuffers.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &m_imageViews[i];
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create Vulkan framebuffer.");
    }

    ADORE_INTERNAL_LOG(INFO, "Vulkan Swapchain created.");
}

Swapchain::~Swapchain()
{
    vkDeviceWaitIdle(m_device);

    for (auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
  
    for (const auto& imageView : m_imageViews)
        vkDestroyImageView(m_device, imageView, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

static unsigned int suitability(VkPhysicalDevice const& device)
{
    unsigned int score = 0;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;
    
    score += deviceProperties.limits.maxImageDimension2D;

    return score;
}

VkSurfaceFormatKHR chooseFormat(VkPhysicalDevice const& physicalDevice, VkSurfaceKHR const& m_surface)
{
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, formats.data());

    for (const auto& format : formats)
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB
         && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;

    return formats[0];
}

VulkanWindow::VulkanWindow(std::shared_ptr<Adore::Context>& ctx, std::string const& title)
    : Window(ctx, title)
{
    VulkanContext * context = static_cast<VulkanContext*>(m_ctx.get());
    surface(context->instance(), &m_surface);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context->instance(), &deviceCount, nullptr);

    if (deviceCount == 0)
        throw Adore::AdoreException("No Vulkan devices found.");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(context->instance(), &deviceCount, devices.data());

    std::multimap<unsigned int, VkPhysicalDevice> suitabilities;

    for (auto const& device : devices)
        suitabilities.emplace(suitability(device), device);

    m_physicalDevice = suitabilities.rbegin()->second;
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
    // ADORE_INTERNAL_LOG(INFO, "Physical Device: " + std::string(deviceProperties.deviceName));

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &presentSupport);

        if (presentSupport) m_queueIndices.present = i;
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) m_queueIndices.graphics = i;
        i++;
    }

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    std::set<uint32_t> uniqueFamilies = { m_queueIndices.graphics, m_queueIndices.present };

    float priority = 1.0f;
    for (const auto& family : uniqueFamilies)
    {
        VkDeviceQueueCreateInfo queueInfo {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = family;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;

        queueInfos.push_back(queueInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {};

    std::vector<char const*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef __APPLE__
    deviceExtensions.emplace_back("VK_KHR_portability_subset");
#endif

    ADORE_INTERNAL_LOG(INFO, "Device Extensions:\n" + vec_to_string(deviceExtensions));

    VkDeviceCreateInfo deviceInfo {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = queueInfos.size();
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.ppEnabledLayerNames = context->layers().data();
    deviceInfo.enabledLayerCount = context->layers().size();
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceInfo.enabledExtensionCount = deviceExtensions.size();

    if (vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create a Vulkan device.");

    vkGetDeviceQueue(m_device, m_queueIndices.graphics, 0, &m_queues.graphics);
    vkGetDeviceQueue(m_device, m_queueIndices.present, 0, &m_queues.present);
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_capabilities);

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
    m_presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, m_presentModes.data());

    m_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode : m_presentModes)
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            m_mode = presentMode;

    m_format = chooseFormat(m_physicalDevice, m_surface);

    if (m_capabilities.maxImageCount == 0) m_imageCount = m_capabilities.minImageCount + 1;
    else m_imageCount = std::clamp(m_capabilities.minImageCount + 1,
                                 m_capabilities.minImageCount,
                                 m_capabilities.maxImageCount);

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_format.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan renderpass.");

    recreateSwapchain();

    ADORE_INTERNAL_LOG(INFO, "Window created (Vulkan).");
}

VulkanWindow::~VulkanWindow()
{
    VulkanContext * context = reinterpret_cast<VulkanContext*>(m_ctx.get());

    m_swapchain.reset();

    vkDestroyRenderPass(m_device, m_renderPass, nullptr); 
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(context->instance(), m_surface, nullptr);
}

void VulkanWindow::recreateSwapchain()
{
    this->framebufferSize(m_extent.width, m_extent.height);
    m_extent.width = std::clamp(m_extent.width, m_capabilities.minImageExtent.width,
                                            m_capabilities.maxImageExtent.width);
    m_extent.height = std::clamp(m_extent.height, m_capabilities.minImageExtent.height,
                                                m_capabilities.maxImageExtent.height);

    m_swapchain = std::make_unique<Swapchain>(m_device, m_surface, m_format, m_mode, m_imageCount, m_extent,
                                              std::vector<uint32_t>{ m_queueIndices.graphics,
                                                                     m_queueIndices.present },
                                              m_renderPass);
}