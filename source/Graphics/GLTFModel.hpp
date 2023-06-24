#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Device.hpp"
#include "Core/Buffer.hpp"
#include "Core/Descriptors.hpp"
#include "Graphics/Texture.hpp"
#include "Scene/Components.hpp"

#include <tinygltf/tiny_gltf.h>

#define MAX_NUM_JOINTS 128u

namespace Nyxis
{
	struct Node;

	struct ShaderValuesParams {
		glm::vec4 lightDir{};
		float exposure = 4.5f;
		float gamma = 2.2f;
		float lod = 1.0f;
		float prefilteredCubeMipLevels;
		float scaleIBLAmbient = 1.0f;
		float debugViewInputs = 0;
		float debugViewEquation = 0;
	};

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
	};

	struct UBOMatrix {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::vec3 camPos;
		float mousePosX;
		float mousePosY;
		int entityID;
	};

	struct Textures {
		TextureCubeMap environmentCube;
		Texture2D empty;
		Texture2D lutBrdf;
		TextureCubeMap irradianceCube;
		TextureCubeMap prefilteredCube;
	};

	struct SceneInfo{
		Textures textures;
		ShaderValuesParams shaderValuesParams;
	};

	struct BoundingBox {
		glm::vec3 min;
		glm::vec3 max;
		bool valid = false;
		BoundingBox();
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox getAABB(glm::mat4 m); // AABB -> axis-aligned bounding box
	};

	struct TextureSampler {
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerAddressMode addressModeU;
		VkSamplerAddressMode addressModeV;
		VkSamplerAddressMode addressModeW;
	};

	struct ModelTexture {
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;
		void updateDescriptor();
		void destroy();
		// Load a texture from a glTF image (stored as vector of chars loaded via stb_image) and generate a full mip chaing for it
		void fromglTFImage(tinygltf::Image& gltfimage, TextureSampler textureSampler);
	};
	
	struct Material {
		std::string name = "defaultMaterial";
		enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
		AlphaMode alphaMode = ALPHAMODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		glm::vec4 emissiveFactor = glm::vec4(1.0f);
		ModelTexture* baseColorTexture;
		ModelTexture* metallicRoughnessTexture;
		ModelTexture* normalTexture;
		ModelTexture* occlusionTexture;
		ModelTexture* emissiveTexture;
		bool doubleSided = false;
		struct TexCoordSets {
			uint8_t baseColor = 0;
			uint8_t metallicRoughness = 0;
			uint8_t specularGlossiness = 0;
			uint8_t normal = 0;
			uint8_t occlusion = 0;
			uint8_t emissive = 0;
		} texCoordSets;
		struct Extension {
			ModelTexture* specularGlossinessTexture;
			ModelTexture* diffuseTexture;
			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			glm::vec3 specularFactor = glm::vec3(0.0f);
		} extension;
		struct PbrWorkflows {
			bool metallicRoughness = true;
			bool specularGlossiness = false;
		} pbrWorkflows;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	};
	
	struct Primitive {
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t vertexCount;
		Material& material;
		bool hasIndices;
		BoundingBox bb;
		Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material& material);
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	struct Mesh {
		Device& device = Device::Get();
		std::vector<Primitive*> primitives;
		BoundingBox bb;
		BoundingBox aabb;
		std::unique_ptr<Buffer> buffer = nullptr;
		struct UniformBuffer {
			Scope<Buffer> meshBuffer;
			VkDescriptorBufferInfo descriptor;
			VkDescriptorSet descriptorSet;
		} uniformBuffer;
		struct UniformBlock {
			glm::mat4 matrix;
			glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
			float jointcount{ 0 };
			uint32_t id{ 0 };
		} uniformBlock;
		Mesh(glm::mat4 matrix, uint32_t id);
		~Mesh();
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	struct Skin {
		std::string name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node*> joints;
	};

	struct Node {
		Node* parent;
		uint32_t index;
		std::vector<Node*> children;
		glm::mat4 matrix;
		std::string name;
		Mesh* mesh;
		Skin* skin;
		int32_t skinIndex = -1;
		glm::vec3 translation{};
		glm::vec3 scale{ 1.0f };
		glm::quat rotation{};
		BoundingBox bvh;
		BoundingBox aabb;
		
		glm::mat4 localMatrix();
		glm::mat4 getMatrix();
		void update();
		~Node();
	};

	struct AnimationChannel {
		enum PathType { TRANSLATION, ROTATION, SCALE };
		PathType path;
		Node* node;
		uint32_t samplerIndex;
	};

	struct AnimationSampler {
		enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct Animation {
		std::string name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	struct Model {
		std::string path = "None";
		bool animate = true;
		bool ready = false;

		struct Vertex {
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv0;
			glm::vec2 uv1;
			glm::vec4 joint0;
			glm::vec4 weight0;
			glm::vec4 color;
		};

		Scope<Buffer> vertexBuffer = nullptr;
		Scope<Buffer> indexBuffer = nullptr;

		glm::mat4 aabb;
		glm::mat4 modelMatrix{ 1.0f };

		std::vector<Node*> nodes;
		std::vector<Node*> linearNodes;

		std::vector<Skin*> skins;

		std::vector<ModelTexture> textures;
		std::vector<TextureSampler> textureSamplers;
		std::vector<Material> materials;
		std::vector<Animation> animations;
		std::vector<std::string> extensions;

		struct Dimensions {
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
		} dimensions;

		struct LoaderInfo {
			uint32_t* indexBuffer;
			Vertex* vertexBuffer;
			size_t indexPos = 0;
			size_t vertexPos = 0;
		};

		uint32_t animationIndex = 0;
		float animationTimer = 0.0f;

		std::vector<Ref<Buffer>> uniformBuffers;
		std::vector<VkDescriptorSet> descriptorSets;

		Model();
		Model(const std::string& filename);
		~Model();

		void destroy();
		void loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, LoaderInfo& loaderInfo, float globalscale);
		void getNodeProps(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount);
		void loadSkins(tinygltf::Model& gltfModel);
		void loadTextures(tinygltf::Model& gltfModel);
		VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
		VkFilter getVkFilterMode(int32_t filterMode);
		void loadTextureSamplers(tinygltf::Model& gltfModel);
		void loadMaterials(tinygltf::Model& gltfModel);
		void loadAnimations(tinygltf::Model& gltfModel);
		void loadFromFile(std::string filename, float scale = 1.0f);
		void bind(VkCommandBuffer commandBuffer);
		void drawNode(Node* node, VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		void calculateBoundingBox(Node* node, Node* parent);
		void getSceneDimensions();
		void updateAnimation(float deltaTime);
		Node* findNode(Node* parent, uint32_t index);
		Node* nodeFromIndex(uint32_t index);
		void updateModelMatrix(TransformComponent& transform);
		void setupDescriptorSet(SceneInfo& sceneInfo, std::vector<Ref<Buffer>>& shaderValuesBuffer);
		void setupNodeDescriptorSet(const Node* node);
		void updateUniformBuffer(uint32_t index, UBOMatrix* ubo);
		VkDescriptorSet getDescriptorSet(uint32_t index) { return descriptorSets[index]; }
	};

	class ModelDescriptorManager
	{
	public:
		static Ref<DescriptorSetLayout> GetModelDescriptorSetLayout();
		static Ref<DescriptorSetLayout> GetMaterialDescriptorSetLayout();
		static Ref<DescriptorSetLayout> GetNodeDescriptorSetLayout();
	private:
		friend struct Model;
		friend class GLTFRenderer;
		static Ref<DescriptorPool> GetDescriptorPool();
		static void Setup();

		inline static Ref<DescriptorPool> m_DescriptorPool = nullptr;
		inline static Ref<DescriptorSetLayout> m_ModelDescriptorSetLayout = nullptr;
		inline static Ref<DescriptorSetLayout> m_MaterialDescriptorSetLayout = nullptr;
		inline static Ref<DescriptorSetLayout> m_NodeDescriptorSetLayout = nullptr;

		inline static bool m_SetupState = false;
	};
}
