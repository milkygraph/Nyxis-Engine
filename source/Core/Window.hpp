#pragma once
#include "Core/Nyxispch.hpp"
#include "Events/Event.hpp"

namespace Nyxis
{
    class Window {
    public:
		Window(int width, int height, const std::string &name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        VkExtent2D GetExtent() { return {static_cast<uint32_t>(m_Data.pWidth), static_cast<uint32_t>(m_Data.pHeight)}; }
        bool ShouldClose() { return glfwWindowShouldClose(m_Data.window); }
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        bool WindowResized() { return m_Data.framebufferResized; }
        void ResetWindowResizedFlag() { m_Data.framebufferResized = false; };

        static GLFWwindow* GetGLFWwindow() { return s_Instance->m_Data.window; }
		static Window & Get(int width, int height, const std::string& name)
		{
			s_Instance = new Window(width, height, name);
			return *s_Instance;
		}

		static Window & Get()
        {
			return *s_Instance;
        }

		using EventCallbackFn = std::function<void(Event&)>;
	    void SetEventCallback(const EventCallbackFn& func){ m_Data.callback = func; }

	private:
		static inline Window* s_Instance = nullptr;
        static void frameBufferResizedCallback(GLFWwindow *window, int width, int height);

		struct GLFWData
		{
			GLFWwindow* window;
			int pWidth;
			int pHeight;
			std::string pTitle;
			bool framebufferResized = false;

			EventCallbackFn callback;
		};
    	EventCallbackFn callback;

		GLFWData m_Data;
    };
}
