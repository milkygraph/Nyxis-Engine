#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Descriptors.hpp"
#include "Core/Window.hpp"
#include "Core/Device.hpp"
#include "Core/Renderer.hpp"
#include "Core/Layer.hpp"
#include "Core/Log.hpp"
#include "Graphics/GameObject.hpp"
#include "Graphics/GLTFModel.hpp"
#include "Graphics/PhysicsEngine.hpp"
#include "Scene/Scene.hpp"
#include "NyxisUI/EditorLayer.hpp"

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
        EditorLayer m_EditorLayer{ pScene };
		bool PhysicsEnabled = false;


        Ref<DescriptorPool> globalPool{};
        Ref<DescriptorSetLayout> globalSetLayout{};
    	std::vector<VkDescriptorSet> globalDescriptorSets;

		Ref<DescriptorPool> imguiPool{};

	    std::vector<std::unique_ptr<Buffer>> uboBuffers;


        GameObject::Map gameObjects;

        Scene pScene{};
        PhysicsEngine physicsEngine{};
        LayerStack pLayerStack{};

	protected:
	    App();
	    std::vector<std::future<void>>futures;
    }; // class App
} // namespace Nyxis
