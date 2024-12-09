#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Log.hpp>
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

VkShaderStageFlagBits stage(Adore::Shader::Type const& type)
{
    switch (type)
    {
        case Adore::Shader::Type::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case Adore::Shader::Type::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
    }
}

VulkanShader::VulkanShader(std::shared_ptr<Adore::Window>& win,
                std::vector<std::pair<Adore::Shader::Type, std::string>> const& modules)
    : Adore::Shader(win)
{
    VulkanWindow* pwindow = static_cast<VulkanWindow*>(m_win.get());

    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(pwindow->device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw Adore::AdoreException("Failed to create pipeline layout.");

    std::vector<VkPipelineShaderStageCreateInfo> shaderInfos(modules.size());

    for (unsigned int i = 0; i < modules.size(); i++)
    {
        VkPipelineShaderStageCreateInfo pipelineInfo;
        pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineInfo.stage = stage(modules[i].first);
        pipelineInfo.module = shader(pwindow->device(), read(modules[i].second));
        pipelineInfo.pName = "main";
        shaderInfos[i] = pipelineInfo;
    }

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
               [](const auto& pair) { return pair.second.c_str(); });

    ADORE_INTERNAL_LOG(INFO, "Shader created:\n" + vec_to_string(shader_paths));
}

VulkanShader::~VulkanShader()
{
    VulkanWindow * window = static_cast<VulkanWindow*>(m_win.get());
    vkQueueWaitIdle(window->queues().graphics);
    vkDestroyPipeline(window->device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(window->device(), m_pipelineLayout, nullptr);
}