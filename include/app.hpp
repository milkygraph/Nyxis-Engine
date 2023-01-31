#pragma once
#include "descriptors.hpp"
#include "window.hpp"
#include "device.hpp"
#include "model.hpp"
#include "gameObject.hpp"
#include "renderer.hpp"
#include "frameInfo.hpp"
#include "scene.hpp"
#include "layer.hpp"
#include "input.hpp"
#include "Log.hpp"

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_structs.hpp>

#include <future>

namespace ve
{

    class App
    {
    public:
		static App* getInstance()
		{
            Log::init();
			pInstance = new App();
            return pInstance;
		}
        ~App();

        // copy constructor and destructors

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        static constexpr int HEIGHT = 800;
        static constexpr int WIDTH = 800;

	    void OnEvent(Event& e);

        void run();
        void init_imgui(VkCommandBuffer commandBuffer);
        void render_imgui(FrameInfo& frameInfo);
        static void close_imgui();

		static veWindow& getWindow() { return pInstance->pWindow; }
	    static veDevice& getDevice() { return pInstance->pDevice; }
		static GLFWwindow* getGLFWwindow() { return pInstance->pWindow.getGLFWwindow(); }
		static Scene& getScene() { return pInstance->pScene; }
    private:
		static App* pInstance;
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        std::pair<std::string, entt::entity> addGameObject(const std::string &path);

        bool newObject = false;

        veWindow& pWindow = veWindow::get(WIDTH, HEIGHT, "Vulkan Engine");
        veDevice& pDevice = veDevice::get();
        veRenderer pRenderer{pScene};

        // TODO: Create third(or first) person Player View
        // std::unique_ptr<veGameObject> Player;

        std::unique_ptr<veDescriptorPool> globalPool{};
        std::unique_ptr<veDescriptorPool> imguiPool{};
        veGameObject::Map gameObjects;

        Scene pScene{};
        LayerStack pLayerStack{};

	protected:
	    App();
	    std::vector<std::future<void>>futures;
    }; // class App
} // namespace ve
