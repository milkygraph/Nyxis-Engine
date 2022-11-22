#pragma once
#include "descriptors.hpp"
#include "window.hpp"
#include "device.hpp"
#include "model.hpp"
#include "gameObject.hpp"
#include "renderer.hpp"
#include "frameInfo.hpp"
#include "scene.hpp"

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>
namespace ve
{

    class App
    {
    public:
        App();
        ~App();

        // copy constructor and destructors

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        static constexpr int HEIGHT = 800;
        static constexpr int WIDTH = 800;

        void run();
        void init_imgui(VkCommandBuffer commandBuffer);
        void render_imgui(FrameInfo& frameInfo);
        void close_imgui();

    private:
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        id_t addGameObject(std::string& model);

        bool newObject = false;

        veWindow pWindow{WIDTH, HEIGHT, "VulkanApp"};
        veDevice pDevice{pWindow};
        veRenderer pRenderer{pWindow, pDevice};

        // TODO: Create third(or first) person Player View
        // std::unique_ptr<veGameObject> Player;

        std::unique_ptr<veDescriptorPool> globalPool{};
        std::unique_ptr<veDescriptorPool> imguiPool{};
        veGameObject::Map gameObjects;

        Scene m_Scene;
    }; // class App
} // namespace ve
