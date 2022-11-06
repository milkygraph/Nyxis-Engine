#include "pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <cassert>
#include <vulkan/vulkan_core.h>

namespace ve
{
    vePipeline::vePipeline(veDevice &device, const std::string &vertPath, const std::string &fragPath, const PipelineConfigInfo &configInfo)
        : device{device}
    {

        createGraphicsPipeline(vertPath, fragPath, configInfo);
    }

    vePipeline::~vePipeline()
    {
        vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
        vkDestroyPipeline(device.device(), graphicsPipeline, nullptr);
    }

    std::vector<char> vePipeline::readFile(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    void vePipeline::createGraphicsPipeline(const std::string &vertPath, const std::string &fragPath, const PipelineConfigInfo &config)
    {
        assert(config.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in config");
        assert(config.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in config");

        auto vertCode = readFile(vertPath);
        auto fragCode = readFile(fragPath);

        createShaderModule(vertCode, &vertShaderModule);
        createShaderModule(fragCode, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &config.viewport; // this and following line can cause a problem when viewportInfo gets copied
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &config.scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo; // this should be local variable because of possible issues with copying viewportinfo
        pipelineInfo.pRasterizationState = &config.rasterizationInfo;
        pipelineInfo.pMultisampleState = &config.multisamplingInfo;
        pipelineInfo.pColorBlendState = &config.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &config.depthStencilInfo;
        pipelineInfo.pDynamicState = nullptr; // will be added later

        pipelineInfo.layout = config.pipelineLayout;
        pipelineInfo.renderPass = config.renderPass;
        pipelineInfo.subpass = config.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to successfully create graphics pipeline");
        }
    }

    void vePipeline::createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!");
        }
    }
    PipelineConfigInfo vePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height)
    {
        PipelineConfigInfo config{};
        config.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        config.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        config.viewport.x = 0.0f;
        config.viewport.y = 0.0f;
        config.viewport.width = static_cast<float>(width);
        config.viewport.height = static_cast<float>(height);
        config.viewport.maxDepth = 1.0f;
        config.viewport.minDepth = 0.0f;

        config.scissor.offset = {0, 0};
        config.scissor.extent = {width, height};

        config.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        config.rasterizationInfo.depthClampEnable = VK_FALSE;
        config.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        config.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        config.rasterizationInfo.lineWidth = 1.0f;
        config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        config.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        config.rasterizationInfo.depthBiasEnable = VK_FALSE;
        config.rasterizationInfo.depthBiasConstantFactor = 0.0f;
        config.rasterizationInfo.depthBiasClamp = 0.0f;
        config.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        config.multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        config.multisamplingInfo.sampleShadingEnable = VK_FALSE;
        config.multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        config.multisamplingInfo.minSampleShading = 1.0f;
        config.multisamplingInfo.pSampleMask = nullptr;
        config.multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
        config.multisamplingInfo.alphaToOneEnable = VK_FALSE;

        config.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        config.colorBlendAttachment.blendEnable = VK_FALSE;
        config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // I guess this has something to do with when two objects overlap
        config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // basically the first line make the source to complete overlap the dst
        config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // this setting should be changes for objects like glass from which you can see within
        config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // we will come back to this once we start working with transparency
        config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        config.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        config.colorBlendInfo.logicOpEnable = VK_FALSE;
        config.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        config.colorBlendInfo.attachmentCount = 1;
        config.colorBlendInfo.pAttachments = &config.colorBlendAttachment;
        config.colorBlendInfo.blendConstants[0] = 0.0f;
        config.colorBlendInfo.blendConstants[1] = 0.0f;
        config.colorBlendInfo.blendConstants[2] = 0.0f;
        config.colorBlendInfo.blendConstants[3] = 0.0f;

        config.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        config.depthStencilInfo.depthTestEnable = VK_TRUE;
        config.depthStencilInfo.depthWriteEnable = VK_TRUE;
        config.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        config.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        config.depthStencilInfo.stencilTestEnable = VK_FALSE;
        config.depthStencilInfo.minDepthBounds = 0.0f;
        config.depthStencilInfo.maxDepthBounds = 1.0f;
        config.depthStencilInfo.front = {};
        config.depthStencilInfo.back = {};

        return config;
    }
}