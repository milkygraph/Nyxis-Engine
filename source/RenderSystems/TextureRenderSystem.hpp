#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Pipeline.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Descriptors.hpp"

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

		Device &pDevice = Device::Get();
		std::unique_ptr<vePipeline> pPipeline;
		VkPipelineLayout m_PipelineLayout;
		
		std::vector<Ref<DescriptorPool>> m_TexturePool{};
		std::vector<VkDescriptorSet> m_TextureDescriptorSets;
		Ref<DescriptorSetLayout> m_TextureSetLayout;
	};
}