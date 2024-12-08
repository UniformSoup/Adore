#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Vulkan/Shader.hpp>

VulkanRenderer::VulkanRenderer(std::shared_ptr<Adore::Window>& win)
    : Adore::Renderer(win)
{
    VulkanWindow* window = static_cast<VulkanWindow*>(m_win.get());

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = window->format().format;
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

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(window->device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan renderpass.");

    m_framebuffers.resize(window->imageViews().size());

    for (size_t i = 0; i < window->imageViews().size(); i++)
    {
        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &window->imageViews()[i];
        framebufferInfo.width = window->extent().width;
        framebufferInfo.height = window->extent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(window->device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create Vulkan framebuffer.");
    }

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = window->queueIndices().graphics;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(window->device(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan command pool.");

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(window->device(), &allocInfo, &m_commandBuffer) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to allocate Vulkan command buffer.");

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(window->device(), &semaphoreInfo, nullptr, &m_imageAvailable) != VK_SUCCESS
    ||  vkCreateSemaphore(window->device(), &semaphoreInfo, nullptr, &m_renderFinished) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan semaphores.");
    
    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateFence(window->device(), &fenceInfo, nullptr, &m_inFlight) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan fence.");

    ADORE_INTERNAL_LOG(INFO, "Created Renderer (Vulkan).");
}

VulkanRenderer::~VulkanRenderer()
{
    auto window = static_cast<VulkanWindow*>(m_win.get());

    vkDestroyFence(window->device(), m_inFlight, nullptr);

    vkDestroySemaphore(window->device(), m_imageAvailable, nullptr);
    vkDestroySemaphore(window->device(), m_renderFinished, nullptr);

    vkDestroyCommandPool(window->device(), m_commandPool, nullptr);
    
    for (auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(window->device(), framebuffer, nullptr);

    vkDestroyRenderPass(window->device(), m_renderPass, nullptr);   
}

void VulkanRenderer::render(std::shared_ptr<Adore::Shader>& shader)
{
    auto window = static_cast<VulkanWindow*>(m_win.get());
    auto pshader = static_cast<VulkanShader*>(shader.get());

    vkWaitForFences(window->device(), 1, &m_inFlight, VK_TRUE, UINT64_MAX);
    vkResetFences(window->device(), 1, &m_inFlight);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(window->device(), window->swapchain(),
                          UINT64_MAX, m_imageAvailable, VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(m_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to begin Vulkan command buffer.");

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = window->extent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pshader->pipeline());

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(window->extent().width);
    viewport.height = static_cast<float>(window->extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = window->extent();

    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    vkCmdDraw(m_commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(m_commandBuffer);
    vkEndCommandBuffer(m_commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailable;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderFinished;

    if (vkQueueSubmit(window->queues().graphics, 1, &submitInfo, m_inFlight) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to submit Vulkan command buffer.");

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderFinished;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &window->swapchain();
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(window->queues().present, &presentInfo);
}