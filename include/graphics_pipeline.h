#pragma once
#include <string_view>
#include <Volk/volk.h>
#include <shaderc/shaderc.hpp>
#include <glm/glm.hpp>

#include "utils.h"
#include "swapchain.h"
#include "geometry.h"


class GraphicsPipeline {
public:
    GraphicsPipeline(
        Device& device,
        Swapchain& swapchain
    );
    ~GraphicsPipeline();

    VkPipeline getGraphicsPipelineHandle() const { return graphicsPipelineHandle; }
    VkPipelineLayout getGraphicsPipelineLayout() const { return graphicsPipelineLayout; }
private: 
    VkShaderModule createShaderModule(
        const std::string_view  filepath, 
        shaderc_shader_kind     shaderKind
    );

    Device& device;

    VkShaderModule vertShaderModule = nullptr;
    VkShaderModule fragShaderModule = nullptr;
    VkPipelineLayout graphicsPipelineLayout = nullptr;
    VkPipeline graphicsPipelineHandle = nullptr;
};