#include <Adore/Internal/Vulkan/Context.hpp>
#include <Adore/Version.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Window.hpp>

#include <GLFW/glfw3.h>

#ifdef DEBUG
    constexpr static bool debug = true;
#else
    constexpr static bool debug = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL validationCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
{
    Adore::Severity s;
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            s = Adore::Severity::ERROR;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            s = Adore::Severity::WARN;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            s = Adore::Severity::INFO;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            s = Adore::Severity::INFO;
            break;
        default:
            return VK_FALSE;
    }

    log_message
    (
        {
            purple + "Adore" + reset,
            purple + "Vulkan" + reset,
            sevcolors[s] + sevstrings[s] + reset
        },
        sevcolors[s] + pCallbackData->pMessage + reset
    );

    return VK_FALSE;
}

VulkanContext::VulkanContext(std::string const& appName)
    : Context(Adore::API::Vulkan)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "Adore";
    appInfo.engineVersion = VK_MAKE_VERSION(ADORE_VERSION_MAJOR, ADORE_VERSION_MINOR, ADORE_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    m_extensions = Window::requiredInstanceExtensions();

    if (debug)
    {
        m_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        m_layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

#ifdef __APPLE__
    m_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    m_extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    ADORE_INTERNAL_LOG(INFO, "Instance Extensions:\n" + vec_to_string(m_extensions));
    ADORE_INTERNAL_LOG(INFO, "Validation Layers:\n" + vec_to_string(m_layers));
    
    instanceInfo.enabledExtensionCount = m_extensions.size();
    instanceInfo.ppEnabledExtensionNames = m_extensions.data();
    instanceInfo.enabledLayerCount = m_layers.size();
    instanceInfo.ppEnabledLayerNames = m_layers.data();

    if (vkCreateInstance(&instanceInfo, nullptr, &m_instance) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create a Vulkan instance.");

    VkDebugUtilsMessengerCreateInfoEXT messengerInfo {};
    messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerInfo.pfnUserCallback = validationCallback;
    messengerInfo.pUserData = nullptr;
    
    if (debug)
    {
        auto vkCreateDebugUtilsMessengerEXT
                = (PFN_vkCreateDebugUtilsMessengerEXT)
                    vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
        
        if (vkCreateDebugUtilsMessengerEXT(m_instance, &messengerInfo, nullptr, &m_messenger) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create a Vulkan debug messenger.");
    }

    ADORE_INTERNAL_LOG(INFO, "Context Created (Vulkan).");
}

VulkanContext::~VulkanContext()
{
    if (debug)
    {
        auto vkDestroyDebugUtilsMessengerEXT
                = (PFN_vkDestroyDebugUtilsMessengerEXT)
                    vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
        
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_messenger, nullptr);
    }

    vkDestroyInstance(m_instance, nullptr);
}