#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Log.hpp>

#include <map>
#include <set>
#include <algorithm>

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

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

    VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode : presentModes)
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            mode = presentMode;

    m_format = chooseFormat(m_physicalDevice, m_surface);
    m_extent = capabilities.currentExtent;

    if (m_extent.width == UINT32_MAX || m_extent.height == UINT32_MAX)
    {
        this->framebufferSize(m_extent.width, m_extent.height);
        m_extent.width = std::clamp(m_extent.width, capabilities.minImageExtent.width,
                                                capabilities.maxImageExtent.width);
        m_extent.height = std::clamp(m_extent.height, capabilities.minImageExtent.height,
                                                  capabilities.maxImageExtent.height);
    }

    uint32_t imageCount;

    if (capabilities.maxImageCount == 0) imageCount = capabilities.minImageCount + 1;
    else imageCount = std::clamp(capabilities.minImageCount + 1,
                                 capabilities.minImageCount,
                                 capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapchainInfo {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = m_surface;
    swapchainInfo.minImageCount = imageCount;
    swapchainInfo.imageFormat = m_format.format;
    swapchainInfo.imageColorSpace = m_format.colorSpace;
    swapchainInfo.imageExtent = m_extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (m_queueIndices.graphics != m_queueIndices.present)
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[] = { m_queueIndices.graphics, m_queueIndices.present };
        swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0;
        swapchainInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainInfo.preTransform = capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = mode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create a Vulkan swapchain.");

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

    m_imageViews.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format.format;
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };

        if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create a Vulkan image view.");
    }

    ADORE_INTERNAL_LOG(INFO, "Window created (Vulkan).");
}

VulkanWindow::~VulkanWindow()
{
    VulkanContext * context = reinterpret_cast<VulkanContext*>(m_ctx.get());

    vkDeviceWaitIdle(m_device);
    
    for (const auto& imageView : m_imageViews)
        vkDestroyImageView(m_device, imageView, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroySurfaceKHR(context->instance(), m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
}