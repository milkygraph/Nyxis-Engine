#pragma once
#include "device.hpp"
#include "buffer.hpp"
#include "texture.hpp"

#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "vepch.hpp"

namespace ve
{
    class veModel
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

        veModel(const std::string& filepath);
        ~veModel();

        veModel(const veModel &) = delete;
        veModel &operator=(const veModel &) = delete;

//        static std::shared_ptr<veModel> createModelFromFile(const std::string& filepath);

		bool loaded = false;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer) const;
		void loadModel();

		static std::shared_ptr<veModel> CreateModel(const std::string& filepath)
		{
			if (models.find(filepath) == models.end())
			{
				models[filepath] = std::make_shared<veModel>(filepath);
			    
            }
			return models[filepath];
		}

		using ModelMap = std::unordered_map<std::string, std::shared_ptr<veModel>>;

		inline static ModelMap& GetModels() { return models; }

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        bool hasIndexBuffer = false;

		Builder* builder = nullptr;
		veDevice &device = veDevice::get();
        std::unique_ptr<veBuffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<veBuffer> indexBuffer;
        uint32_t indexCount;
        static ModelMap models;
	};
}