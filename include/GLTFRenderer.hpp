#pragma once

#include "Nyxis.hpp"
#include "Nyxispch.hpp"
#include "frameInfo.hpp"
#include "pipeline.hpp"
#include "descriptors.hpp"

#include "GLTFModel.hpp"

namespace Nyxis
{
	class GLTFRenderer
	{
		struct PushConstBlockMaterial {
			glm::vec4 baseColorFactor;
			glm::vec4 emissiveFactor;
			glm::vec4 diffuseFactor;
			glm::vec4 specularFactor;
			float workflow;
			int colorTextureSet;
			int PhysicalDescriptorTextureSet;
			int normalTextureSet;
			int occlusionTextureSet;
			int emissiveTextureSet;
			float metallicFactor;
			float roughnessFactor;
			float alphaMask;
			float alphaMaskCutoff;
		} pushConstBlockMaterial;

		struct DescriptorSetLayouts {
			VkDescriptorSetLayout scene;
			VkDescriptorSetLayout material;
			VkDescriptorSetLayout node;
		} descriptorSetLayouts;

		struct shaderValuesParams {
			glm::vec4 lightDir;
			float exposure = 4.5f;
			float gamma = 2.2f;
			float prefilteredCubeMipLevels;
			float scaleIBLAmbient = 1.0f;
			float debugViewInputs = 0;
			float debugViewEquation = 0;
		} shaderValuesParams;

	public:
		GLTFRenderer(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~GLTFRenderer();

		void OnUpdate();
		void Render(FrameInfo& frameInfo);

	private:
		void CreatePipeline(VkRenderPass renderPass);
		void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void SetupDescriptorSets();

		Device& device = Device::get();
		Scope<vePipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		std::vector<Ref<veDescriptorPool>> m_DescriptorPools;
		std::vector<VkDescriptorSet> m_DescriptorSets;
		Ref<veDescriptorSetLayout> m_DescriptorSetLayout;
	};
}
