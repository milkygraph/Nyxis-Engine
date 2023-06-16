#pragma once
#include "Core/Nyxis.hpp"
#include "Core/Nyxispch.hpp"
#include "Graphics/GLTFModel.hpp"

constexpr auto DEPTH_ARRAY_SCALE = 2048; // will be used fir object picking buffer;

namespace Nyxis
{
	class Pipeline;

	enum class PipelineType
	{
		SKYBOX = 0,
		PBR,
		PBR_DOUBLE_SIDED,
		PBR_ALPHA_BLEND
	};

    struct ObjectPicking
    {
        uint32_t depthBufferObject[DEPTH_ARRAY_SCALE];
        uint32_t selectedEntity = 0;
    };

    struct Pipelines
    {
        Ref<Pipeline> skybox;
        Ref<Pipeline> pbr;
        Ref<Pipeline> pbrDoubleSided;
        Ref<Pipeline> pbrAlphaBlend;
    };

    struct LightSource {
        glm::vec3 color = glm::vec3(1.0f, 0.2f, 0.5f);
        glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
    };

	class GLTFRenderer
	{
	public:
		static inline SceneInfo s_SceneInfo{};
        static inline PushConstBlockMaterial s_PushConstBlockMaterial;

		static inline UBOMatrix s_ShaderValuesScene{}, s_ShaderValuesSkybox{};
		static inline bool s_PBRPipelineUpdate = false;
		static inline bool s_SkyboxPipelineUpdate = false;

		static void Init(VkRenderPass vk_render_pass);
		static void Shutdown();

		static void OnUpdate();
		static void Render();
		static void UpdateAnimation(float dt);
		static void UpdatePipeline(PipelineType type);
		static void LoadEnvironment(std::string& filename);
		static void UpdateSkyboxDescriptorSets();
		static void UpdateScene() { s_SceneUpdated = true; }
		static void OnEvent(const Event& event);

		static inline std::string s_EnvMapFile = "";
		static inline bool s_SceneUpdated = false;
		static inline bool s_Animate = false;

		static inline std::vector<Ref<Buffer>> s_SkyboxBuffers{};
		static inline std::vector<Ref<Buffer>> s_UniformBuffersParams{};
		static inline std::vector<Ref<Buffer>> s_ObjectPickingBuffer{};

        static inline LightSource lightSource{};
		static inline ObjectPicking objectPicking{};
        static inline Pipelines Pipes{};

	private:
		static void PrepareUniformBuffers();
		static void UpdateBuffers();
		static void LoadAssets();
		static void GenerateBRDFLUT();
		static void GenerateCubemaps();
		static void PreparePipelines(VkRenderPass renderPass);
		static void SetupDescriptorPool();
		static void SetupDescriptorSets();
		static void FreeDescriptorSets();
		static void RenderNode(Node* node, Material::AlphaMode alphaMode, Model& model);

		static inline Device* device{};

		enum PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSINESS = 1 };

		static inline struct Pipelines2
		{
			VkPipeline pbr;
			VkPipeline pbrDoubleSided;
			VkPipeline pbrAlphaBlend;
		} pipelines;

		static inline VkPipeline boundPipeline = VK_NULL_HANDLE;
		static inline VkPipelineLayout pipelineLayout;
		static inline VkPipelineCache pipelineCache = VK_NULL_HANDLE;

		static inline VkDescriptorPool descriptorPool;

		static inline VkDescriptorSetLayout depthBufferLayout;

		static inline std::vector<VkDescriptorSet> skyboxDescriptorSets;
		static inline std::vector<VkDescriptorSet> depthBufferDescriptorSets;

		static inline Ref<Model> skybox = nullptr;
		static inline std::thread animationThread{};
	};
}
