#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Pipeline.hpp"

namespace Nyxis
{
	class RenderSystem
	{
	public:
		RenderSystem(std::string& VertexShaderPath, std::string& FragmentShaderPath, VkRenderPass RenderPass);
		virtual ~RenderSystem();

		virtual void OnUpdate();
		virtual void Render();
		virtual void Cleanup();

		virtual void CreatePipelineLayout();
		virtual void CreatePipeline(VkRenderPass renderPass);
		virtual void CreateDescriptorSetLayout();

                Device & pDevice = Device::Get();
		std::unique_ptr<vePipeline> pPipeline;
		VkPipelineLayout pipelineLayout;

		std::string& VertexShaderPath;
		std::string& FragmentShaderPath;

		VkDescriptorPool DescriptorPool;
		std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
		std::vector<VkDescriptorSet> DescriptorSets;
	};
}