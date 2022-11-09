#pragma once
#include "window.hpp"
#include "device.hpp"
#include "model.hpp"
#include "gameObject.hpp"
#include "renderer.hpp"

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
    struct SimplePushConstantData
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };


    class App
    {
    public:
        App();
        ~App();

        // copy constructor and destructors

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        static constexpr int HEIGHT = 600;
        static constexpr int WIDTH = 800;

        void run();

    private:
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);

        veWindow pWindow{WIDTH, HEIGHT, "VulkanApp"};
        veDevice pDevice{pWindow};
        veRenderer pRenderer{pWindow, pDevice};

        std::vector<veGameObject> gameObjects;
    }; // class App
} // namespace ve
