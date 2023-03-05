#pragma once

#include "Nyxis.hpp"
#include "Nyxispch.hpp"
#include "frameInfo.hpp"
#include "pipeline.hpp"
#include "scene.hpp"

#include "GLTFModel.hpp"

namespace Nyxis
{
	class GLTFRenderer
	{
	public:
		struct Textures {
			TextureCubeMap environmentCube;
			Texture2D empty;
			Texture2D lutBrdf;
			Ref<TextureCubeMap> irradianceCube = nullptr;
			Ref<TextureCubeMap> prefilteredCube = nullptr;
		} textures;

		struct Models
		{
			Model scene;
			Model skybox;
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

		struct UBOMatrices {
			glm::mat4 projection;
			glm::mat4 model;
			glm::mat4 view;
			glm::vec3 camPos;
		} shaderValuesScene, shaderValuesSkybox;

		struct shaderValuesParams {
			glm::vec4 lightDir;
			float exposure = 4.5f;
			float gamma = 2.2f;
			float prefilteredCubeMipLevels;
			float scaleIBLAmbient = 1.0f;
			float debugViewInputs = 0;
			float debugViewEquation = 0;
		} shaderValuesParams;

		GLTFRenderer(VkRenderPass renderPass, VkExtent2D extent);
		~GLTFRenderer();

		void OnUpdate();
		void Render(FrameInfo& frameInfo);
		void LoadEnvironment(std::string& filename);
		void UpdateSkyboxDescriptorSets();

		std::string envMapFile = "";
		bool SceneUpdated = false;
	private:
		void PrepareUniformBuffers();
		void UpdateUniformBuffers(Scene& scene);
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
		} descriptorSetLayouts;

		struct DescriptorSets
		{
			VkDescriptorSet scene;
			VkDescriptorSet skybox;
		};

		struct UniformBufferSet {
			Ref<Buffer> scene = nullptr;
			Ref<Buffer> skybox = nullptr;
			Ref<Buffer> params = nullptr;
		};

		std::vector<DescriptorSets> descriptorSets;
		std::vector<UniformBufferSet> uniformBuffers;

		struct LightSource {
			glm::vec3 color = glm::vec3(1.0f, 0.2f, 0.5f);
			glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
		} lightSource;
	
	};
}
