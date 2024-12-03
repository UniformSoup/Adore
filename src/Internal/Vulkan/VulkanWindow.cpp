#include <Adore/Internal/Vulkan/VulkanWindow.hpp>
#include <Adore/Internal/Log.hpp>

#include <map>
#include <set>

struct QueueIndices
{
    uint32_t graphics;
    uint32_t present;
    QueueIndices populate(VkPhysicalDevice const& device, VkSurfaceKHR const& surface);
};

QueueIndices QueueIndices::populate(VkPhysicalDevice const& device, VkSurfaceKHR const& surface)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) this->present = i;
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) this->graphics = i;
        i++;
    }

    return *this;
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

VulkanWindow::VulkanWindow(std::shared_ptr<VulkanContext>& ctx, std::string const& title)
    : Window(title), m_ctx(ctx)
{
    surface(m_ctx->instance(), &m_surface);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_ctx->instance(), &deviceCount, nullptr);

    if (deviceCount == 0)
        throw Adore::AdoreException("No Vulkan devices found.");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_ctx->instance(), &deviceCount, devices.data());

    std::multimap<unsigned int, VkPhysicalDevice> suitabilities;

    for (auto const& device : devices)
        suitabilities.emplace(suitability(device), device);

    const auto physicalDevice = suitabilities.rbegin()->second;
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    // ADORE_INTERNAL_LOG(INFO, "Physical Device: " + std::string(deviceProperties.deviceName));

    const auto indices = QueueIndices{}.populate(physicalDevice, m_surface);

    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    std::set<uint32_t> uniqueFamilies = { indices.graphics, indices.present };

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

    VkDeviceCreateInfo deviceInfo {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = queueInfos.size();
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.ppEnabledLayerNames = m_ctx->layers().data();
    deviceInfo.enabledLayerCount = m_ctx->layers().size();
#ifdef __APPLE__
    std::vector<char const*> deviceExtensions = { "VK_KHR_portability_subset" };
    deviceInfo.enabledExtensionCount = deviceExtensions.size();
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
#else
    deviceInfo.enabledExtensionCount = 0;
    deviceInfo.ppEnabledExtensionNames = nullptr;
#endif

    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &m_device) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create a Vulkan device.");

    vkGetDeviceQueue(m_device, indices.graphics, 0, &queues.graphics);
    vkGetDeviceQueue(m_device, indices.present, 0, &queues.present);

    ADORE_INTERNAL_LOG(INFO, "Window created (Vulkan).");
}

VulkanWindow::~VulkanWindow()
{
    vkDestroySurfaceKHR(m_ctx->instance(), m_surface, nullptr);
    vkDestroyDevice(m_device, nullptr);
}