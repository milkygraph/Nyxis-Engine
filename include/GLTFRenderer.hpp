#pragma once

#include "Nyxis.hpp"
#include "Nyxispch.hpp"
#include "frameInfo.hpp"
#include "pipeline.hpp"
#include "scene.hpp"

#include "GLTFModel.hpp"

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

		UBOMatrices shaderValuesScene{}, shaderValuesSkybox{};

		GLTFRenderer(VkRenderPass renderPass);
		~GLTFRenderer();

		void OnUpdate(Scene& scene);
		void Render(FrameInfo& frameInfo);
		void UpdateAnimation(FrameInfo& frameInfo);
		void LoadEnvironment(std::string& filename);
		void UpdateSkyboxDescriptorSets();

		std::string envMapFile = "";
		bool SceneUpdated = false;
		bool animate = true;

		RigidBody rigidBody{};
		struct UniformBufferSet {
			Ref<Buffer> skybox = nullptr;
		};
		std::vector<UniformBufferSet> uniformBuffers;
		std::vector<Ref<Buffer>> uniformBuffersParams;
		std::vector<Ref<Buffer>> depthBuffers;

		uint32_t depthBufferObject[DEPTH_ARRAY_SCALE];

	private:
		void PrepareUniformBuffers();
		void UpdateUniformBuffers(FrameInfo& frameInfo);
		void LoadModel(std::string& filename);
		void SetScene(const Ref<Scene> scene) { this->scene = scene; }
		void LoadAssets();
		void GenerateBRDFLUT();
		void GenerateCubemaps();
		void PreparePipelines(VkRenderPass renderPass);
		void SetupDescriptorPool();
		void SetupNodeDescriptorSet(const Node* node);
		void SetupDescriptorSets();
		void FreeDescriptorSets();
		void RenderNode(Node* node, FrameInfo& frameInfo, Material::AlphaMode alphaMode);
		void RenderNodeImproved(Node* node, FrameInfo& frameInfo, Material::AlphaMode alphaMode, Model& model);

		Device& device = Device::get();
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

		int32_t animationIndex = 0;
		float animationTimer = 0.0f;
	};
}
