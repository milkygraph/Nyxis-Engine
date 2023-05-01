#pragma once
#include "Core/Camera.hpp"
#include "Graphics/GameObject.hpp"
#include "Scene/Components.hpp"

namespace Nyxis
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
        }
    };

    struct FrameInfo
    {
        int frameIndex = 0;
        float frameTime = 0;
		glm::vec2 mousePosition = { 0, 0 };
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;
        GameObject::Map gameObjects;
    };
}