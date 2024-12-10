#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/FramesInFlight.hpp>
#include <Adore/Internal/Vulkan/Buffer.hpp>

#include <fstream>

std::vector<uint32_t> read(std::string const& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw Adore::AdoreException("Failed to open shader file: " + path);
    std::vector<uint32_t> buffer(file.tellg());

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    return buffer;
}

VkShaderModule shader(VkDevice const& device, std::vector<uint32_t> const& code)
{
    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = code.size();
    moduleCreateInfo.pCode = code.data();

    VkShaderModule module;
    if (vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &module) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create shader module.");

    return module;
}

VkShaderStageFlagBits stage(Adore::ShaderType const& type)
{
    switch (type)
    {
        case Adore::ShaderType::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case Adore::ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
}

VkFormat format(Adore::AttributeFormat const& type)
{
    switch (type)
    {
        case Adore::AttributeFormat::FLOAT:       return VK_FORMAT_R32_SFLOAT;
        case Adore::AttributeFormat::VEC2_FLOAT:  return VK_FORMAT_R32G32_SFLOAT;
        case Adore::AttributeFormat::VEC3_FLOAT:  return VK_FORMAT_R32G32B32_SFLOAT;
        case Adore::AttributeFormat::VEC4_FLOAT:  return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Adore::AttributeFormat::INT:         return VK_FORMAT_R32_SINT;
        case Adore::AttributeFormat::VEC2_INT:    return VK_FORMAT_R32G32_SINT;
        case Adore::AttributeFormat::VEC3_INT:    return VK_FORMAT_R32G32B32_SINT;
        case Adore::AttributeFormat::VEC4_INT:    return VK_FORMAT_R32G32B32A32_SINT;
        case Adore::AttributeFormat::UINT:        return VK_FORMAT_R32_UINT;
        case Adore::AttributeFormat::VEC2_UINT:   return VK_FORMAT_R32G32_UINT;
        case Adore::AttributeFormat::VEC3_UINT:   return VK_FORMAT_R32G32B32_UINT;
        case Adore::AttributeFormat::VEC4_UINT:   return VK_FORMAT_R32G32B32A32_UINT;
        case Adore::AttributeFormat::DOUBLE:      return VK_FORMAT_R64_SFLOAT;
        case Adore::AttributeFormat::VEC2_DOUBLE: return VK_FORMAT_R64G64_SFLOAT;
        case Adore::AttributeFormat::VEC3_DOUBLE: return VK_FORMAT_R64G64B64_SFLOAT;
        case Adore::AttributeFormat::VEC4_DOUBLE: return VK_FORMAT_R64G64B64A64_SFLOAT;
    }
}

VulkanShader::VulkanShader(std::shared_ptr<Adore::Window>& win,
                std::vector<Adore::ShaderModule> const& modules,
                Adore::LayoutDescriptor const& descriptor,
                std::vector<std::shared_ptr<Adore::UniformBuffer>> const& uniformBuffers)
    : Adore::Shader(win, descriptor, uniformBuffers)
{
    VulkanWindow* pwindow = static_cast<VulkanWindow*>(m_win.get());

    std::vector<VkDescriptorSetLayoutBinding> uniformDescriptions(m_descriptor.uniforms.size());

    for (unsigned int i = 0; i < m_descriptor.uniforms.size(); i++)
    {
        uniformDescriptions[i].binding = m_descriptor.uniforms[i].binding;
        uniformDescriptions[i].descriptorType = (m_descriptor.uniforms[i].type == Adore::UniformType::BUFFER)
                            ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        uniformDescriptions[i].descriptorCount = m_descriptor.uniforms[i].count;
        uniformDescriptions[i].stageFlags = stage(m_descriptor.uniforms[i].stage);
        uniformDescriptions[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = uniformDescriptions.size();
    layoutInfo.pBindings = uniformDescriptions.data();

    if (vkCreateDescriptorSetLayout(pwindow->device(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan descriptor set layout.");

    std::vector<VkDescriptorPoolSize> poolSizes(m_descriptor.uniforms.size());

    for (unsigned int i = 0; i < m_descriptor.uniforms.size(); i++)
    {
        poolSizes[i].type = (m_descriptor.uniforms[i].type == Adore::UniformType::BUFFER)
                            ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[i].descriptorCount = FRAMES_IN_FLIGHT;
    }

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(pwindow->device(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan descriptor pool.");

    std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(pwindow->device(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to allocate Vulkan descriptor sets.");


    std::vector<VkDescriptorBufferInfo> bufferInfos(m_descriptor.uniforms.size() * FRAMES_IN_FLIGHT);
    std::vector<VkWriteDescriptorSet> writes(m_descriptor.uniforms.size() * FRAMES_IN_FLIGHT);

    for (unsigned int i = 0; i < uniformBuffers.size(); i++)
    {
        for (unsigned int j = 0; j < FRAMES_IN_FLIGHT; j++)
        {
            bufferInfos[i * FRAMES_IN_FLIGHT + j].buffer = static_cast<VulkanUniformBuffer*>(uniformBuffers[i].get())->buffer(j);
            bufferInfos[i * FRAMES_IN_FLIGHT + j].offset = 0;
            bufferInfos[i * FRAMES_IN_FLIGHT + j].range = VK_WHOLE_SIZE;

            writes[i * FRAMES_IN_FLIGHT + j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i * FRAMES_IN_FLIGHT + j].dstSet = m_descriptorSets[j];
            writes[i * FRAMES_IN_FLIGHT + j].dstBinding = m_descriptor.uniforms[i].binding;
            writes[i * FRAMES_IN_FLIGHT + j].dstArrayElement = 0;
            writes[i * FRAMES_IN_FLIGHT + j].descriptorCount = 1;
            writes[i * FRAMES_IN_FLIGHT + j].descriptorType = (m_descriptor.uniforms[i].type == Adore::UniformType::BUFFER)
                                ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i * FRAMES_IN_FLIGHT + j].pBufferInfo = &bufferInfos[i * FRAMES_IN_FLIGHT + j];
            writes[i * FRAMES_IN_FLIGHT + j].pImageInfo = nullptr;
            writes[i * FRAMES_IN_FLIGHT + j].pTexelBufferView = nullptr;
        }
    }

    vkUpdateDescriptorSets(pwindow->device(), writes.size(), writes.data(), 0, nullptr);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(pwindow->device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create pipeline layout.");

    std::vector<VkPipelineShaderStageCreateInfo> shaderInfos(modules.size());

    for (unsigned int i = 0; i < modules.size(); i++)
    {
        VkPipelineShaderStageCreateInfo pipelineInfo;
        pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineInfo.stage = stage(modules[i].type);
        pipelineInfo.module = shader(pwindow->device(), read(modules[i].path));
        pipelineInfo.pName = "main";
        shaderInfos[i] = pipelineInfo;
    }

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(m_descriptor.attributes.size());
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(m_descriptor.bindings.size());

    for (unsigned int i = 0; i < m_descriptor.bindings.size(); i++)
    {
        bindingDescriptions[i].binding = m_descriptor.bindings[i].binding;
        bindingDescriptions[i].stride = m_descriptor.bindings[i].stride;
        bindingDescriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    }

    for (unsigned int i = 0; i < m_descriptor.attributes.size(); i++)
    {
        attributeDescriptions[i].binding = m_descriptor.attributes[i].binding;
        attributeDescriptions[i].location = m_descriptor.attributes[i].location;
        attributeDescriptions[i].format = format(m_descriptor.attributes[i].format);
        attributeDescriptions[i].offset = m_descriptor.attributes[i].offset;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)  pwindow->extent().width;
    viewport.height = (float) pwindow->extent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = { 0, 0 };
    scissor.extent = pwindow->extent();

    VkPipelineViewportStateCreateInfo viewportStateInfo {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizerInfo {};
    rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerInfo.depthClampEnable = VK_FALSE;
    rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerInfo.depthBiasEnable = VK_FALSE;
    rasterizerInfo.depthBiasConstantFactor = 0.0f;
    rasterizerInfo.depthBiasClamp = 0.0f;
    rasterizerInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleInfo {};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.sampleShadingEnable = VK_FALSE;
    multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo.minSampleShading = 1.0f;
    multisampleInfo.pSampleMask = nullptr;
    multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState blendAttachmentInfo {};
    blendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentInfo.blendEnable = VK_FALSE;
    blendAttachmentInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentInfo.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendAttachmentInfo.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendingInfo {};
    colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendingInfo.logicOpEnable = VK_FALSE;
    colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendingInfo.attachmentCount = 1;
    colorBlendingInfo.pAttachments = &blendAttachmentInfo;

    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = pwindow->format().format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderInfos.size();
    pipelineInfo.pStages = shaderInfos.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizerInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pColorBlendState = &colorBlendingInfo;
    pipelineInfo.pDynamicState = &dynamicStateInfo;

    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = pwindow->renderpass();
    pipelineInfo.subpass = 0;
    pipelineInfo.pDepthStencilState = nullptr;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    if (vkCreateGraphicsPipelines(pwindow->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create Vulkan graphics pipeline.");

    for (auto& info : shaderInfos)
        vkDestroyShaderModule(pwindow->device(), info.module, nullptr);

    std::vector<const char*> shader_paths;

    std::transform(modules.begin(), modules.end(), std::back_inserter(shader_paths),
               [](const auto& pair) { return pair.path.c_str(); });

    ADORE_INTERNAL_LOG(INFO, "Shader created:\n" + vec_to_string(shader_paths));
}

VulkanShader::~VulkanShader()
{
    VulkanWindow * window = static_cast<VulkanWindow*>(m_win.get());
    vkQueueWaitIdle(window->queues().graphics);
    vkDestroyPipeline(window->device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(window->device(), m_pipelineLayout, nullptr);
    vkDestroyDescriptorPool(window->device(), m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(window->device(), m_descriptorSetLayout, nullptr);
}