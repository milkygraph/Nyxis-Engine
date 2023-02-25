#pragma once

#include "Nyxispch.hpp"
#include "renderer.hpp"
#include "pipeline.hpp"
#include "RenderSystem.hpp"
#include "frameInfo.hpp"

namespace Nyxis
{
	class TextureRenderSystem
	{
	public:
		TextureRenderSystem(VkRenderPass RenderPass, VkDescriptorSetLayout globalDescriptorSetLayout);
		~TextureRenderSystem();

		void OnUpdate();
		void Render(FrameInfo& frameInfo);
		void Cleanup();
	private:
		void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void CreatePipeline(VkRenderPass renderPass);

		Device &pDevice = Device::get();
		std::unique_ptr<vePipeline> pPipeline;
		VkPipelineLayout m_PipelineLayout;
		
		std::vector<std::unique_ptr<veDescriptorPool>> m_TexturePool{};
		std::vector<VkDescriptorSet> m_TextureDescriptorSets;
		std::unique_ptr<veDescriptorSetLayout> m_TextureSetLayout;
	};
}