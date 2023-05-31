#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Device.hpp"

namespace Nyxis
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisamplingInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline
    {
    public:
        Pipeline(const std::string &vertPath, const std::string &fragPath, const PipelineConfigInfo &config);
        ~Pipeline();

        // copy constructors and destructors

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        static void DefaultPipelineConfigInfo(PipelineConfigInfo &config);
        static void EnableBlending(PipelineConfigInfo& config);
        static std::vector<char> ReadFile(const std::string& filename);


    	VkGraphicsPipelineCreateInfo pipelineCreateInfo;

    private:

        void CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);
        void CreateGraphicsPipeline(const std::string &vertPath, const std::string &fragPath, const PipelineConfigInfo &config);

        Device &device = Device::Get();
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;

        PipelineConfigInfo pipelineConfigInfo;
    };

} // namespace Nyxis
