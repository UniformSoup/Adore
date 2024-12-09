#pragma once

#include <Adore/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

#include <vulkan/vulkan.h>

class VulkanShader : public Adore::Shader
{
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
public:
    VulkanShader(std::shared_ptr<Adore::Window>& win,
            std::vector<Adore::Shader::Module> const& modules,
            Adore::Shader::InputDescriptor const& descriptor);
    ~VulkanShader();
    VkPipeline const& pipeline() const { return m_pipeline; };
};