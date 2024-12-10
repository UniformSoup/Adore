#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Buffer.hpp>

#include <Adore/Internal/FramesInFlight.hpp>

VulkanRenderer::VulkanRenderer(std::shared_ptr<Adore::Window>& win)
    : Adore::Renderer(win)
{
    VulkanWindow* window = static_cast<VulkanWindow*>(m_win.get());

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = window->queueIndices().graphics;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(window->device(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan command pool.");

    m_commandBuffers.resize(FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = m_commandBuffers.size();

    if (vkAllocateCommandBuffers(window->device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to allocate Vulkan command buffer.");

    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    m_framesAvailable.resize(FRAMES_IN_FLIGHT);
    m_framesRendered.resize(FRAMES_IN_FLIGHT);
    m_framesInFlight.resize(FRAMES_IN_FLIGHT);

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(window->device(), &semaphoreInfo, nullptr, &m_framesAvailable[i]) != VK_SUCCESS
        ||  vkCreateSemaphore(window->device(), &semaphoreInfo, nullptr, &m_framesRendered[i]) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create Vulkan semaphores.");

        if (vkCreateFence(window->device(), &fenceInfo, nullptr, &m_framesInFlight[i]) != VK_SUCCESS)
            throw Adore::AdoreException("Failed to create Vulkan fence.");
    }

    ADORE_INTERNAL_LOG(INFO, "Created Renderer (Vulkan).");
}

VulkanRenderer::~VulkanRenderer()
{
    auto window = static_cast<VulkanWindow*>(m_win.get());

    vkQueueWaitIdle(window->queues().graphics);

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(window->device(), m_framesAvailable[i], nullptr);
        vkDestroySemaphore(window->device(), m_framesRendered[i], nullptr);
        vkDestroyFence(window->device(), m_framesInFlight[i], nullptr);
    }

    vkDestroyCommandPool(window->device(), m_commandPool, nullptr);
}

void VulkanRenderer::begin(std::shared_ptr<Adore::Shader>& shader)
{
    if (m_win != shader->window())
        throw Adore::AdoreException("Shader was not created with the same Window as the Renderer.");

    auto pwindow = static_cast<VulkanWindow*>(m_win.get());
    auto pshader = static_cast<VulkanShader*>(shader.get());

    vkWaitForFences(pwindow->device(), 1, &m_framesInFlight[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(pwindow->device(), 1, &m_framesInFlight[m_currentFrame]);

    m_swapchainImage.first = vkAcquireNextImageKHR(pwindow->device(), pwindow->swapchain().get(),
                UINT64_MAX, m_framesAvailable[m_currentFrame], VK_NULL_HANDLE, &m_swapchainImage.second);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to begin Vulkan command buffer.");

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pwindow->renderpass();
    renderPassInfo.framebuffer = pwindow->swapchain().framebuffers()[m_swapchainImage.second];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = pwindow->extent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffers[m_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pshader->pipeline());

    vkCmdBindDescriptorSets(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pshader->layout(),
                            0, 1, &pshader->descriptorSets()[m_currentFrame], 0, nullptr);

    for (auto& uniform : pshader->uniforms())
        static_cast<VulkanUniformBuffer*>(uniform.get())->update(m_currentFrame);
}

void VulkanRenderer::draw(uint32_t const& count)
{
    auto pwindow = static_cast<VulkanWindow*>(m_win.get());

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pwindow->extent().width);
    viewport.height = static_cast<float>(pwindow->extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = pwindow->extent();

    vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);

    vkCmdDraw(m_commandBuffers[m_currentFrame], count, 1, 0, 0);
}

void VulkanRenderer::drawIndexed(uint32_t const& count)
{
    auto pwindow = static_cast<VulkanWindow*>(m_win.get());

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(pwindow->extent().width);
    viewport.height = static_cast<float>(pwindow->extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = pwindow->extent();

    vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);

    vkCmdDrawIndexed(m_commandBuffers[m_currentFrame], count, 1, 0, 0, 0);
}

void VulkanRenderer::end()
{
    auto pwindow = static_cast<VulkanWindow*>(m_win.get());

    vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]);
    vkEndCommandBuffer(m_commandBuffers[m_currentFrame]);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_framesAvailable[m_currentFrame];
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_framesRendered[m_currentFrame];

    if (vkQueueSubmit(pwindow->queues().graphics, 1, &submitInfo, m_framesInFlight[m_currentFrame]) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to submit Vulkan command buffer.");

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_framesRendered[m_currentFrame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &pwindow->swapchain().get();
    presentInfo.pImageIndices = &m_swapchainImage.second;

    vkQueuePresentKHR(pwindow->queues().present, &presentInfo);

    if (m_swapchainImage.first == VK_ERROR_OUT_OF_DATE_KHR || m_swapchainImage.first == VK_SUBOPTIMAL_KHR)
        pwindow->recreateSwapchain();
    
    m_currentFrame = (m_currentFrame + 1) % FRAMES_IN_FLIGHT;
}

void VulkanRenderer::copy(VkBuffer const& src, VkBuffer const& dst, uint64_t const& size)
{
    VulkanWindow* pwindow = static_cast<VulkanWindow*>(m_win.get());
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(pwindow->device(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to begin Vulkan command buffer.");

    VkBufferCopy copyRegion {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to end Vulkan command buffer.");

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(pwindow->queues().graphics, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(pwindow->queues().graphics);

    vkFreeCommandBuffers(pwindow->device(), m_commandPool, 1, &commandBuffer);
}

void VulkanRenderer::bind(std::shared_ptr<Adore::IndexBuffer>& buffer)
{
    if (buffer->renderer().get() != this)
        throw Adore::AdoreException("Index Buffer is not bound to this renderer.");

    vkCmdBindIndexBuffer(m_commandBuffers[m_currentFrame],
                         static_cast<VulkanIndexBuffer*>(buffer.get())->buffer(),
                         0, VK_INDEX_TYPE_UINT16);
}

void VulkanRenderer::bind(std::shared_ptr<Adore::VertexBuffer>& buffer, uint32_t const& binding)
{
    if (buffer->renderer().get() != this)
        throw Adore::AdoreException("Vertex Buffer is not bound to this renderer.");

    VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], binding, 1,
                           &static_cast<VulkanVertexBuffer*>(buffer.get())->buffer(),
                           &offset);
}

// void VulkanRenderer::bind(std::shared_ptr<Adore::UniformBuffer>& buffer, uint32_t const& binding)
// {
//     if (buffer->renderer().get() != this)
//         throw Adore::AdoreException("Vertex Buffer is not bound to this renderer.");

//     VkDeviceSize offset = 0;

//     vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], binding, 1,
//                            &static_cast<VulkanUniformBuffer*>(buffer.get())->buffer(m_currentFrame),
//                            &offset);
// }