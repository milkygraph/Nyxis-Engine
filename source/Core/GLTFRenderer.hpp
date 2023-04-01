#pragma once
#include "Core/Nyxis.hpp"
#include "Core/Nyxispch.hpp"
#include "Core/Application.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Pipeline.hpp"
#include "Scene/Scene.hpp"
#include "Graphics/GLTFModel.hpp"

#define DEPTH_ARRAY_SCALE 32 // will be used fir object picking buffer

namespace Nyxis
{
	class GLTFRenderer
	{
	public:
		SceneInfo sceneInfo;

		struct Models
		{
			Ref<Model> scene;
			Ref<Model> skybox;
		} models;

		PushConstBlockMaterial pushConstBlockMaterial;

		UBOMatrix shaderValuesScene{}, shaderValuesSkybox{};

		GLTFRenderer(VkRenderPass renderPass);
		~GLTFRenderer();

		void OnUpdate();
		void Render();
		void UpdateAnimation(float dt);
		void LoadEnvironment(std::string& filename);
		void UpdateSkyboxDescriptorSets();

		std::string m_EnvMapFile = "";
		bool m_SceneUpdated = false;
		bool m_Animate = false;

		std::vector<Ref<Buffer>> skyboxBuffers;
		std::vector<Ref<Buffer>> uniformBuffersParams;
		std::vector<Ref<Buffer>> depthBuffers;

		uint32_t depthBufferObject[DEPTH_ARRAY_SCALE];

	private:
		void PrepareUniformBuffers();
		void UpdateUniformBuffers();
		void SetScene() { this->scene = Application::GetScene(); }
		void LoadAssets();
		void GenerateBRDFLUT();
		void GenerateCubemaps();
		void PreparePipelines(VkRenderPass renderPass);
		void SetupDescriptorPool();
		void SetupDescriptorSets();
		void FreeDescriptorSets();
		void RenderNode(Node* node, Material::AlphaMode alphaMode, Model& model);

		Device& device = Device::Get();
		Ref<Scene> scene;

		enum PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSINESS = 1 };

		struct Pipelines
		{
			VkPipeline skybox;
			VkPipeline pbr;
			VkPipeline pbrDoubleSided;
			VkPipeline pbrAlphaBlend;
		} pipelines;

		VkPipeline boundPipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout;
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;

		VkDescriptorPool descriptorPool;

		struct DescriptorSetLayouts
		{
			VkDescriptorSetLayout scene;
			VkDescriptorSetLayout material;
			VkDescriptorSetLayout node;
			VkDescriptorSetLayout depthBufferLayout;
		} descriptorSetLayouts;

		struct DescriptorSets
		{
			VkDescriptorSet scene;
			VkDescriptorSet skybox;
		};

		std::vector<DescriptorSets> descriptorSets;
		std::vector<VkDescriptorSet> depthBufferDescriptorSets;

		struct LightSource {
			glm::vec3 color = glm::vec3(1.0f, 0.2f, 0.5f);
			glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
		} lightSource;
	};
}
