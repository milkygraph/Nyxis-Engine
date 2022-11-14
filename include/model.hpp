#pragma once
#include "device.hpp"
#include "buffer.hpp"

#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <memory>

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
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };

        veModel(veDevice &device, const veModel::Builder &builder);
        ~veModel();

        veModel(const veModel &) = delete;
        veModel &operator=(const veModel &) = delete;

        static std::unique_ptr<veModel> createModelFromFile(veDevice& device, const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        bool hasIndexBuffer = false;

        veDevice &device;
        std::unique_ptr<veBuffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<veBuffer> indexBuffer;
        uint32_t indexCount;
    };
}