#pragma once

#include <Adore/Context.hpp>

#include <vulkan/vulkan.h>

#include <string>

class VulkanContext : public Adore::Context
{
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_messenger;

    std::vector<const char*> m_layers;
    std::vector<const char*> m_extensions;

public:
    VulkanContext(std::string const& appName);
    ~VulkanContext();

    VkInstance const& instance() const { return m_instance; }
    std::vector<const char*> const& layers() const { return m_layers; }
};