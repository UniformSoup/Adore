#pragma once
#include <Adore/Renderer.hpp>

#include <vector>

#include <vulkan/vulkan.h>

class VulkanRenderer : public Adore::Renderer
{
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_framesAvailable;
    std::vector<VkSemaphore> m_framesRendered;
    std::vector<VkFence> m_framesInFlight;
    uint32_t m_currentFrame = 0;
    std::pair<VkResult, uint32_t> m_swapchainImage;
public:
    VulkanRenderer(std::shared_ptr<Adore::Window>& win);
    ~VulkanRenderer();
    void copy(VkBuffer const& src, VkBuffer const& dst, uint64_t const& size);
    void begin(std::shared_ptr<Adore::Shader>& shader) override;
    void bind(std::shared_ptr<Adore::Buffer>& buffer, uint32_t const& binding) override;
    void draw(uint32_t const& count) override;
    void end() override;
};