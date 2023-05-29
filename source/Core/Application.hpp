#pragma once
#include "Core/Nyxispch.hpp"
#include "Core/Window.hpp"
#include "Core/Device.hpp"
#include "Core/FrameInfo.hpp"
#include "Core/Layer.hpp"
#include "Core/Log.hpp"
#include "Graphics/PhysicsEngine.hpp"
#include "Scene/Scene.hpp"
#include "NyxisUI/EditorLayer.hpp"

namespace Nyxis
{
	class NyxisProject;

	class Application
    {
    public:
		static Application* GetInstance()
		{
            Log::Init();
            if(s_Instance == nullptr)
            	s_Instance = new Application();
            return s_Instance;
		}
        ~Application();

		// copy constructor and operator
        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        void Run();
		static Ref<FrameInfo> GetFrameInfo() { return s_Instance->m_FrameInfo; }
		static Ref<NyxisProject> GetProject() { return s_Instance->m_CurrentProject; }
		static Ref<Scene> GetScene() { return s_Instance->m_Scene; }

		static void SetScene(Ref<Scene> scene) { s_Instance->m_Scene = scene; }
        static void SetProject(Ref<NyxisProject> project) { s_Instance->m_CurrentProject = project; }
    private:
        Application();
    	static inline Application* s_Instance = nullptr;
        void OnEvent(Event& e);

    	Window& m_Window = Window::Get(WIDTH, HEIGHT, "Nyxis Engine");
        Device& m_Device = Device::Get();
        Ref<FrameInfo> m_FrameInfo = nullptr;
        Ref<Scene> m_Scene = nullptr;
        Ref<NyxisProject> m_CurrentProject = nullptr;

        LayerStack m_LayerStack{};
        EditorLayer m_EditorLayer{};
        PhysicsEngine m_PhysicsEngine{};

    }; // class Application
} // namespace Nyxis
