#pragma once

#include "Camera.hpp"
#include "gameObject.hpp"
#include "scene.hpp"
#include "components.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace ve
{

#define MAX_LIGHTS 10

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseViewMatrix{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 1.f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int pointLightCount;
       
        void UpdateVPM(Camera* camera)
        {
			projection = camera->getProjectionMatrix();
			view = camera->getViewMatrix();
			inverseViewMatrix = camera->getInverseViewMatrix();
        }
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalDescriptorSet;
		veDescriptorPool& TexturePool;
        veGameObject::Map &gameObjects;
        Scene& scene;
    };
}