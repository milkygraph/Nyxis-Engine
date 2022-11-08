#pragma once
#include "device.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace ve
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisamplingInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0; // No idead what this is
    };

    class vePipeline
    {
    public:
        vePipeline(veDevice &device, const std::string &vertPath, const std::string &fragPath, const PipelineConfigInfo &config);
        ~vePipeline();

        // copy constructors and destructors

        vePipeline(const vePipeline &) = delete;
        vePipeline &operator=(const vePipeline &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        static void defaultPipelineConfigInfo(PipelineConfigInfo &config, uint32_t width, uint32_t height);

    private:
        std::vector<char> readFile(const std::string &filename);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);
        void createGraphicsPipeline(const std::string &vertPath, const std::string &fragPath, const PipelineConfigInfo &config);

        veDevice &device;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };

} // namespace ve
