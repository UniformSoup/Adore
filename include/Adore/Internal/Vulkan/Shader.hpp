#pragma once

#include <Adore/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

#include <vulkan/vulkan.h>

// Add vertex attrib to shader input.
class VulkanShader : public Adore::Shader
{
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
public:
    VulkanShader(std::shared_ptr<Adore::Window>& win,
            std::vector<std::pair<Adore::Shader::Type, std::string>> const& modules);
    ~VulkanShader();
    VkPipeline const& pipeline() const { return m_pipeline; };
};