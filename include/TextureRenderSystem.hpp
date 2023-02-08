#pragma once

#include "vepch.hpp"
#include "renderer.hpp"
#include "pipeline.hpp"
#include "RenderSystem.hpp"
#include "frameInfo.hpp"

namespace ve
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

		veDevice &pDevice = veDevice::get();
		std::unique_ptr<vePipeline> pPipeline;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<veDescriptorSetLayout> TextureSetLayout;
	};
}