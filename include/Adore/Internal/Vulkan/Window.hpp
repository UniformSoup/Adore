#pragma once
#include <Adore/Internal/Window.hpp>
#include <Adore/Internal/Vulkan/Context.hpp>

#include <memory>


// Check for swapchain support.
// Add options for VSync / VK_PRESENT_MODE_XXX's
// Make swapchain into class and resize() reset the swapchain.
// (call Window::resize(...); swapchain->rebuild();)

class Swapchain
{
    VkDevice const& m_device;

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    VkSwapchainKHR m_swapchain;
public:
    Swapchain(VkDevice const& device, VkSurfaceKHR const& surface, VkSurfaceFormatKHR const& format,
                     VkPresentModeKHR const& mode, uint32_t imageCount, VkExtent2D const& extent,
                     std::vector<uint32_t> const& queueIndices, VkRenderPass const& renderPass); // remove queueIndices later.
    ~Swapchain();
    VkSwapchainKHR const& get() const { return m_swapchain; };
    // std::vector<VkImageView> const& imageViews() const { return m_imageViews; };
    std::vector<VkFramebuffer> const& framebuffers() const { return m_framebuffers; };
};

class VulkanWindow : public Window
{
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkRenderPass m_renderPass;
    VkSurfaceCapabilitiesKHR m_capabilities;
    VkSurfaceFormatKHR m_format;
    VkExtent2D m_extent;
    VkPresentModeKHR m_mode;
    uint32_t m_imageCount;

    std::vector<VkPresentModeKHR> m_presentModes;
    std::unique_ptr<Swapchain> m_swapchain;

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
    Queues const& queues() const { return m_queues; };
    QueueIndices const& queueIndices() const { return m_queueIndices; };
    Swapchain const& swapchain() const { return *m_swapchain.get(); };
    VkExtent2D const& extent() const { return m_extent; };
    VkSurfaceFormatKHR const& format() const { return m_format; };
    void recreateSwapchain();
    VkRenderPass const& renderpass() const { return m_renderPass; };
    VkPhysicalDevice const& physicalDevice() const { return m_physicalDevice; };
};