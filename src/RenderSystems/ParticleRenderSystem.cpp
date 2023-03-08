#include "RenderSystems\ParticleRenderSystem.hpp"

namespace Nyxis
{
	ParticleRenderSystem::ParticleRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
	{
		// create descriptor pool
		m_ParticlePool.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		m_ParticleSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		auto poolBuilder = veDescriptorPool::Builder()
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		for(auto& pool : m_ParticlePool)
		{
			pool = poolBuilder.build();
		}
		
		CreatePipelineLayout(globalSetLayout);
		CreatePipeline(renderPass);
		
		model = veModel::CreateModel("../models/circle.obj");
	}

	ParticleRenderSystem::~ParticleRenderSystem()
	{}

	void ParticleRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		// a descriptor set layout with global ubo and a particles buffer
		m_ParticleSetLayout = veDescriptorSetLayout::Builder()
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
			.build();
		
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalSetLayout,
			m_ParticleSetLayout->getDescriptorSetLayout()
		};
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		
		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ParticleRenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		PipelineConfigInfo pipelineConfig{};
		vePipeline::defaultPipelineConfigInfo(pipelineConfig);
		
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();

		//pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_PipelineLayout;
		m_Pipeline = std::make_unique<vePipeline>(
			"../shaders/particle_shader.vert.spv",
			"../shaders/particle_shader.frag.spv",
			pipelineConfig);
	}

	void ParticleRenderSystem::Update(float dt)
	{}

	void ParticleRenderSystem::Render(FrameInfo& frameInfo)
	{
		m_ParticlePool[frameInfo.frameIndex]->resetPool();

		m_Pipeline->bind(frameInfo.commandBuffer);
	
		// write particle buffer to descriptor set
		VkDescriptorSet descriptorSet;
		auto bufferInfo = m_ParticleBuffer->descriptorInfo();
		veDescriptorWriter(*m_ParticleSetLayout, *m_ParticlePool[frameInfo.frameIndex])
			.writeBuffer(0, &bufferInfo)
			.build(m_ParticleSets[frameInfo.frameIndex]);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			0,
			1,
			&frameInfo.globalDescriptorSet,
			0,
			nullptr);
		
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			1,
			1,
			&m_ParticleSets[frameInfo.frameIndex],
			0,
			nullptr);

		//vkCmdDraw(frameInfo.commandBuffer, 1, m_Particles.size(), 0, 0);
		for (auto particle : m_Particles)
		{
			//draw particle
			vkCmdDraw(frameInfo.commandBuffer, 1, 1, 0, 0);
		}
	}
	void ParticleRenderSystem::BuildBuffer()
	{
		Buffer stagingBuffer(sizeof(m_Particles[0]), m_Particles.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(m_Particles.data());

		m_ParticleBuffer = std::make_unique<Buffer>(sizeof(m_Particles[0]), m_Particles.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		
		m_Device.copyBuffer(stagingBuffer.getBuffer(), m_ParticleBuffer->getBuffer(), sizeof(m_Particles[0]) * m_Particles.size());
	}
}