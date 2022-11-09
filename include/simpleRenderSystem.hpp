#pragma once
#include "window.hpp"
#include "pipeline.hpp"
#include "device.hpp"
#include "model.hpp"
#include "gameObject.hpp"
#include "renderer.hpp"
#include "simpleRenderSystem.hpp"

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_glfw.h>

namespace ve
{
    class SimpleRenderSystem
    {
    public:
        SimpleRenderSystem(veDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        // copy constructor and destructors

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;
        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<veGameObject>& gameObjects);


    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        veDevice &pDevice;

        std::unique_ptr<vePipeline> pPipeline; 
        VkPipelineLayout pipelineLayout;
    }; // class SimpleRenderSystem
} // namespace ve
