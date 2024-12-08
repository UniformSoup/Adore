#pragma once
#include <Adore/Renderer.hpp>

#include <vector>

#include <vulkan/vulkan.h>

class VulkanRenderer : public Adore::Renderer
{
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    VkSemaphore m_imageAvailable;
    VkSemaphore m_renderFinished;
    VkFence m_inFlight;
public:
    VulkanRenderer(std::shared_ptr<Adore::Window>& win);
    ~VulkanRenderer();
    VkRenderPass const& renderpass() const { return m_renderPass; };
    void render(std::shared_ptr<Adore::Shader>& shader) override;
};