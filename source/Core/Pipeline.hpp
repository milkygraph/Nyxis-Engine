#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Device.hpp"

namespace Nyxis
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

        std::vector<VkDynamicState> dynamicStateEnables{};
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};

        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline
    {
    public:
        Pipeline(const std::string &vertPath, const std::string &fragPath);
        ~Pipeline();

        // copy constructors and destructors

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void Create();
        void Recreate();
        void Bind(VkCommandBuffer commandBuffer);
        PipelineConfigInfo& GetConfig() { return pipelineConfigInfo; }
    	static void DefaultPipelineConfigInfo(PipelineConfigInfo &config);
        static void EnableBlending(PipelineConfigInfo& config);
        static std::vector<char> ReadFile(const std::string& filename);

    	PipelineConfigInfo pipelineConfigInfo;
    private:

        void CreateShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        Device &device = Device::Get();
        VkPipeline graphicsPipeline;

        std::string vertPath;
        std::string fragPath;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
} // namespace Nyxis
