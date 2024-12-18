#pragma once

#include <Adore/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

#include <vulkan/vulkan.h>


class VulkanShader : public Adore::Shader
{
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipeline m_pipeline;
public:
    VulkanShader(std::shared_ptr<Adore::Window>& win,
                std::vector<Adore::ShaderModule> const& modules,
                Adore::LayoutDescriptor const& descriptor);
    ~VulkanShader();
    void attach(std::shared_ptr<Adore::UniformBuffer>& buffer, uint32_t const& binding);
    void attach(std::shared_ptr<Adore::Sampler>& buffer, uint32_t const& binding);
    VkPipeline const& pipeline() const { return m_pipeline; };
    std::vector<VkDescriptorSet> const& descriptorSets() const { return m_descriptorSets; };
    VkPipelineLayout const& layout() const
    {
        return m_pipelineLayout;
    };
};