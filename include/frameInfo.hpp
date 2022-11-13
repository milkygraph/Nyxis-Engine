#pragma once

#include "camera.hpp"
#include "gameObject.hpp"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace ve
{
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