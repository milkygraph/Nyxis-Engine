#pragma once
#include "device.hpp"
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

namespace ve
{
    class veModel
    {
    public:

        struct Vertex{
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        };

        veModel(veDevice &device, const std::vector<Vertex> &vertices);
        ~veModel();

        veModel(const veModel &) = delete;
        veModel &operator=(const veModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);


    private:

        void createVertexBuffers(const std::vector<Vertex> &vertices);


        veDevice &device;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
}