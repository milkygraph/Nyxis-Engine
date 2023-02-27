#include "GLTFRenderer.hpp"

namespace Nyxis
{
	GLTFRenderer::GLTFRenderer(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
	{
		m_DescriptorPools.resize(veSwapChain::MAX_FRAMES_IN_FLIGHT);
		auto poolBuilder = veDescriptorPool::Builder()
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		for (auto& pool : m_DescriptorPools)
		{
			pool = poolBuilder.build();
		}

		CreatePipelineLayout(globalSetLayout);
		CreatePipeline(renderPass);
		SetupDescriptorSets();
	}

	GLTFRenderer::~GLTFRenderer()
	{

	}

	void GLTFRenderer::OnUpdate()
	{

	}

	void GLTFRenderer::Render(FrameInfo& frameInfo)
	{
	}

	void GLTFRenderer::CreatePipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		vePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_PipelineLayout;
		pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		
		pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
		pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
		pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineConfig.depthStencilInfo.front = pipelineConfig.depthStencilInfo.back;
		pipelineConfig.depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

		pipelineConfig.bindingDescriptions = { { 0, sizeof(Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
		pipelineConfig.attributeDescriptions = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 },
			{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10 },
			{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14 },
			{ 6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 18 }
		};

		m_Pipeline = std::make_unique<vePipeline>(
			"../shaders/pbr.vert.spv",
			"../shaders/pbr.frag.spv",
			pipelineConfig);
	}

	void GLTFRenderer::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		m_DescriptorSetLayout = veDescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		const std::vector<VkDescriptorSetLayout> setLayouts = {
			descriptorSetLayouts.scene, descriptorSetLayouts.material, descriptorSetLayouts.node
		};
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		pipelineLayoutCI.pSetLayouts = setLayouts.data();

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.size = sizeof(PushConstBlockMaterial);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pipelineLayoutCI.pushConstantRangeCount = 1;
		pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutCI, nullptr, &m_PipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout!");

	}

	void GLTFRenderer::SetupDescriptorSets()
	{

	}


}
