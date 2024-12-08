#pragma once
#include <Adore/Internal/Window.hpp>
#include <Adore/Internal/Vulkan/Context.hpp>

#include <memory>


// Check for swapchain support.
// Add options for VSync / VK_PRESENT_MODE_XXX's
// Make swapchain into class and resize() reset the swapchain.
// (call Window::resize(...); swapchain->rebuild();)

class VulkanWindow : public Window
{
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkSurfaceFormatKHR m_format;
    VkExtent2D m_extent;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    struct Queues
    {
        VkQueue graphics;
        VkQueue present;
    } m_queues;

    struct QueueIndices
    {
        uint32_t graphics;
        uint32_t present;
    } m_queueIndices;

public:
    VulkanWindow(std::shared_ptr<Adore::Context>& ctx, std::string const& title);
    ~VulkanWindow();
    VkDevice const& device() const { return m_device; };
    VkExtent2D const& extent() const { return m_extent; };
    VkSurfaceFormatKHR const& format() const { return m_format; };
    std::vector<VkImageView> const& imageViews() const { return m_imageViews; };
    Queues const& queues() const { return m_queues; };
    QueueIndices const& queueIndices() const { return m_queueIndices; };
    VkSwapchainKHR const& swapchain() const { return m_swapchain; };
};