#pragma once
#include "device.hpp"
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

namespace ve
{
    struct PipelineConfigInfo
    {
        VkViewport viewport;
        VkRect2D scissor;
        
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{}; // specifying parameters of the input assembly stage
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{}; // specifying parameters of the rasterization stage
        VkPipelineMultisampleStateCreateInfo multisamplingInfo{};   // specifying parameters of the multisampling stage
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;         // specifying parameters of the color blending stage
        VkPipelineColorBlendAttachmentState colorBlendAttachment{}; // specifying parameters of the color blending stage
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};   // specifying parameters of the depth and stencil testing stage
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0; // No idead what this is
    };

    class vePipeline
    {
    public:
        vePipeline(veDevice &device,
                   const std::string &vertPath,
                   const std::string &fragPath,
                   const PipelineConfigInfo &configInfo);
        ~vePipeline();

        // copy constructors and destructors

        vePipeline(const vePipeline &) = delete;
        void operator=(const vePipeline &) = delete;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

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
