#pragma once

#include "camera.hpp"
#include "gameObject.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace ve
{

#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{};    // w is intensity
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseViewMatrix{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int pointLightCount;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        veCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        veGameObject::Map &gameObjects;
    };
}