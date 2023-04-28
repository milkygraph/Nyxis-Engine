#pragma once
#include "Core/Nyxis.hpp"
#include "Core/Nyxispch.hpp"
#include "Core/Application.hpp"
#include "Core/Pipeline.hpp"
#include "Scene/Scene.hpp"
#include "Graphics/GLTFModel.hpp"

constexpr auto DEPTH_ARRAY_SCALE = 32; // will be used fir object picking buffer;

namespace Nyxis
{
	inline SceneInfo* g_SceneInfo;
	inline std::vector<Ref<Buffer>>* g_UniformBufferParams;
	class GLTFRenderer
	{
	public:
		SceneInfo sceneInfo{};

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
		std::vector<Ref<Buffer>> objectPickingBuffer;

		struct ObjectPicking
		{
			uint32_t depthBufferObject[DEPTH_ARRAY_SCALE];
			uint32_t selectedEntity = 0;
		} objectPicking;

	private:
		void PrepareUniformBuffers();
		void UpdateBuffers();
		void LoadAssets();
		void GenerateBRDFLUT();
		void GenerateCubemaps();
		void PreparePipelines(VkRenderPass renderPass);
		void SetupDescriptorPool();
		void SetupDescriptorSets();
		void FreeDescriptorSets();
		void RenderNode(Node* node, Material::AlphaMode alphaMode, Model& model);

		Device& device = Device::Get();

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

		VkDescriptorSetLayout depthBufferLayout;

		std::vector<VkDescriptorSet> skyboxDescriptorSets;
		std::vector<VkDescriptorSet> depthBufferDescriptorSets;

		struct LightSource {
			glm::vec3 color = glm::vec3(1.0f, 0.2f, 0.5f);
			glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
		} lightSource;

		Ref<Model> skybox;
	};
}
