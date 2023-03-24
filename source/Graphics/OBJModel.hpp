#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Device.hpp"
#include "Core/Buffer.hpp"

namespace Nyxis
{
    class OBJModel
    {
    public:
        struct Vertex
        {
            glm::vec3 position{0.0f};
            glm::vec3 color{1.0f, 1.0f, 1.0f};
            glm::vec3 normal{0.0f};
            glm::vec2 uv{0.0f};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        
            bool operator==(const Vertex&other) const{
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder
        {
			Builder(std::string filepath) : filepath{filepath} {}
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
			const std::string filepath;

            void loadModel();
        };

        OBJModel(const std::string& filepath);
        ~OBJModel();

        OBJModel(const OBJModel &) = delete;
        OBJModel &operator=(const OBJModel &) = delete;

		bool loaded = false;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer) const;
		void loadModel();

		static std::shared_ptr<OBJModel> CreateModel(const std::string& filepath)
		{
			if (models.find(filepath) == models.end())
			{
				models[filepath] = std::make_shared<OBJModel>(filepath);
			    
            }
			return models[filepath];
		}

        static void ReleaseModels()
        {
            models.clear();
        }

		using ModelMap = std::unordered_map<std::string, std::shared_ptr<OBJModel>>;

		inline static ModelMap& GetModels() { return models; }

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        bool hasIndexBuffer = false;

		Builder* builder = nullptr;
                Device &device = Device::get();
        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;
        static ModelMap models;
	};
}