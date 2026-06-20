#include "graphics_pipeline.h"

GraphicsPipeline::GraphicsPipeline(
    Device& device, 
    Swapchain& swapchain
) : device(device)
{
    // -- create shaders
    vertShaderModule = createShaderModule(ASSET_DIR "/shader/shader.vert", shaderc_vertex_shader);
    fragShaderModule = createShaderModule(ASSET_DIR "/shader/shader.frag", shaderc_fragment_shader);

    //-- create graphics pipeline
    VkPipelineLayoutCreateInfo graphicsPipelineLayoutCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    if (vkCreatePipelineLayout(device.getLogicalDeviceHandle(), &graphicsPipelineLayoutCreateInfo, nullptr, &graphicsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed creating pipeline layout!");
    }

    const char* entryPoint = "main";

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages {
        VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = entryPoint
        },
        VkPipelineShaderStageCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = entryPoint
        }
    };

    VkVertexInputBindingDescription vertInputBindingDescription {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    std::vector<VkVertexInputAttributeDescription> vertInputAttributeDescriptions {
        VkVertexInputAttributeDescription {
            .location = 0, 
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position)
        },
        VkVertexInputAttributeDescription {
            .location = 1, 
            .binding = 0,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(Vertex, color)
        }
    };

    VkPipelineVertexInputStateCreateInfo vertInputCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertInputBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertInputAttributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo { 
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .stencilTestEnable = VK_FALSE
    };

    VkPipelineViewportStateCreateInfo viewportCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendAttachmentState attachmentState {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, 
    };
    VkPipelineColorBlendStateCreateInfo blendCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachmentState
    };

    std::vector<VkDynamicState> dynamicStates {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkFormat swapchainFormat = swapchain.getSwapchainFormat();
    VkPipelineRenderingCreateInfo renderingCreateInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchainFormat,
        .depthAttachmentFormat = swapchain.getSwapchainDepthFormat()
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .stageCount = static_cast<uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState = &viewportCreateInfo,
        .pRasterizationState = &rasterizationCreateInfo,
        .pMultisampleState = &multiSampleCreateInfo,
        .pDepthStencilState = &depthStencilCreateInfo,
        .pColorBlendState = &blendCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = graphicsPipelineLayout,
        .renderPass = VK_NULL_HANDLE
    };

    if (vkCreateGraphicsPipelines(device.getLogicalDeviceHandle(), nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &graphicsPipelineHandle) != VK_SUCCESS) {
        throw std::runtime_error("failed creating graphics pipeline!");
    }
}

GraphicsPipeline::~GraphicsPipeline()
{
    if (graphicsPipelineLayout) vkDestroyPipelineLayout(device.getLogicalDeviceHandle(), graphicsPipelineLayout, nullptr);
    if (graphicsPipelineHandle) vkDestroyPipeline(device.getLogicalDeviceHandle(), graphicsPipelineHandle, nullptr);
    if (fragShaderModule) vkDestroyShaderModule(device.getLogicalDeviceHandle(), fragShaderModule, nullptr);
    if (vertShaderModule) vkDestroyShaderModule(device.getLogicalDeviceHandle(), vertShaderModule, nullptr);
}

VkShaderModule GraphicsPipeline::createShaderModule(const std::string_view filepath, shaderc_shader_kind shaderKind)
{
    std::string shaderSource = readTextFile(filepath);
    if (shaderSource.empty()) {
        throw std::runtime_error(std::format("shader src file: [{}] is empty!", filepath));
    }

    std::println("compiling shader: {}", filepath);
    shaderc::Compiler compiler;
    shaderc::CompileOptions compileOptions;
    compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    compileOptions.SetTargetSpirv(shaderc_spirv_version_1_6);
    compileOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
    shaderc::CompilationResult result = compiler.CompileGlslToSpv(shaderSource, shaderKind, filepath.data());

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        throw std::runtime_error(std::format("failed compiling shader: [{}]", filepath));
    }

    const size_t shaderSize = (result.cend() - result.cbegin()) * sizeof(uint32_t);

    VkShaderModuleCreateInfo moduleCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderSize,
        .pCode = result.cbegin()
    };

    VkShaderModule shaderModule = nullptr;
    if (vkCreateShaderModule(device.getLogicalDeviceHandle(), &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error(std::format("failed creating shader module: [{}]", filepath));
    }

    return shaderModule;
}
