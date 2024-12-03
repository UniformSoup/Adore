#pragma once
#include <Adore/Internal/Window.hpp>
#include <Adore/Internal/Vulkan/VulkanContext.hpp>

#include <memory>

class VulkanWindow : public Window
{
    std::shared_ptr<VulkanContext> m_ctx;
    VkSurfaceKHR m_surface;
    VkDevice m_device;
    VkSwapchainKHR m_swapchain;

    struct 
    {
        VkQueue graphics;
        VkQueue present;
    } queues;

public:
    VulkanWindow(std::shared_ptr<VulkanContext>& ctx, std::string const& title);
    ~VulkanWindow();
};