#pragma once
#include "Core/Nyxis.hpp"
#include "Core/Nyxispch.hpp"
#include "Core/Buffer.hpp"
#include "Core/Device.hpp"
#include "Core/Pipeline.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Descriptors.hpp"
#include "Scene/Components.hpp"

namespace Nyxis
{
	class ParticleRenderSystem
	{
	public:
		ParticleRenderSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~ParticleRenderSystem();

		void AddParticle(Particle particle) { m_Particles.push_back(particle); }
		void Update(float dt);
		void Render(FrameInfo& frameInfo);
		void BuildBuffer();
	private:
		void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void CreatePipeline(VkRenderPass renderPass);

		Device& m_Device = Device::Get();
		
		std::shared_ptr<OBJModel> model;
		std::vector<Particle> m_Particles;
		std::unique_ptr<Buffer> m_ParticleBuffer;
		VkPipelineLayout m_PipelineLayout;
		std::shared_ptr<vePipeline> m_Pipeline;
		
		std::vector<Ref<DescriptorPool>> m_ParticlePool{};
		Ref<DescriptorSetLayout> m_ParticleSetLayout;

		std::vector<VkDescriptorSet>m_ParticleSets{};
	};
}