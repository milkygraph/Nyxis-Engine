#include "Nyxispch.hpp"
#include "Nyxis.hpp"

#include "buffer.hpp"
#include "device.hpp"
#include "pipeline.hpp"
#include "frameInfo.hpp"
#include "components.hpp"
#include "descriptors.hpp"

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

		Device& m_Device = Device::get();
		
		std::shared_ptr<veModel> model;
		std::vector<Particle> m_Particles;
		std::unique_ptr<Buffer> m_ParticleBuffer;
		VkPipelineLayout m_PipelineLayout;
		std::shared_ptr<vePipeline> m_Pipeline;
		
		std::vector<std::unique_ptr<veDescriptorPool>> m_ParticlePool{};
		std::unique_ptr<veDescriptorSetLayout> m_ParticleSetLayout;

		std::vector<VkDescriptorSet>m_ParticleSets{};
	};
}