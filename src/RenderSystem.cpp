#include "RenderSystem.hpp"

namespace Nyxis
{
	RenderSystem::RenderSystem(std::string& VertexShaderPath, std::string& FragmentShaderPath, VkRenderPass RenderPass)
		: VertexShaderPath(VertexShaderPath), FragmentShaderPath(FragmentShaderPath)
	{
		CreatePipelineLayout();
		CreatePipeline(RenderPass);
	}

	RenderSystem::~RenderSystem()
	{
		vkDestroyPipelineLayout(pDevice.device(), pipelineLayout, nullptr);
	}

	void RenderSystem::OnUpdate() {}

	void RenderSystem::Render() {}

	void RenderSystem::Cleanup() {}

	void RenderSystem::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = DescriptorSetLayouts.data();

		if (vkCreatePipelineLayout(pDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	void RenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		vePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;

		pPipeline = std::make_unique<vePipeline>(VertexShaderPath, FragmentShaderPath, pipelineConfig);
	}
	void RenderSystem::CreateDescriptorSetLayout() {}
}