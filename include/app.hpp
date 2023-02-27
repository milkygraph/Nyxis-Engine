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
#include "ImguiLayer.hpp"
#include "PhysicsEngine.hpp"
#include "GLTFModel.hpp"

#include "Nyxispch.hpp"

namespace Nyxis
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
		void Setup();
		static Window & getWindow() { return pInstance->pWindow; }
	    static Device & getDevice() { return pInstance->pDevice; }
		static GLFWwindow* getGLFWwindow() { return pInstance->pWindow.getGLFWwindow(); }
		static Scene& getScene() { return pInstance->pScene; }
    private:
		static App* pInstance;
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        std::pair<std::string, entt::entity> addGameObject(const std::string &path);

        bool newObject = false;

        Window & pWindow = Window::get(WIDTH, HEIGHT, "Nyxis");
        Device & pDevice = Device::get();
        Renderer pRenderer{ pScene};
        ImguiLayer pImguiLayer{ pScene };
		bool PhysicsEnabled = false;


        std::unique_ptr<veDescriptorPool> globalPool{};
        std::unique_ptr<veDescriptorSetLayout> globalSetLayout{};
		std::vector<VkDescriptorSet> globalDescriptorSets;

		std::unique_ptr<veDescriptorPool> imguiPool{};

	    std::vector<std::unique_ptr<Buffer>> uboBuffers;


        veGameObject::Map gameObjects;
        Model model;

        Scene pScene{};
        PhysicsEngine physicsEngine{};
        LayerStack pLayerStack{};

	protected:
	    App();
	    std::vector<std::future<void>>futures;
    }; // class App
} // namespace Nyxis
