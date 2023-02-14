#pragma once
#include "Events/event.hpp"


#define GLFW_INCLUDE_VULKAN
#include "vepch.hpp"

namespace ve
{
    class Window {
    public:
      Window(int width, int height, const std::string &name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        VkExtent2D getExtent() { return {static_cast<uint32_t>(pData.pWidth), static_cast<uint32_t>(pData.pHeight)}; }
        bool shouldClose() { return glfwWindowShouldClose(pData.window); }
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        bool windowResized() { return pData.framebufferResized; }
        void resetWindowResizedFlag() { pData.framebufferResized = false; };

        static GLFWwindow* getGLFWwindow() { return pInstance->pData.window; }
		static Window & get(int width, int height, const std::string& name)
		{
			pInstance = new Window(width, height, name);
			return *pInstance;
		}

		static Window & get() { return *pInstance; }


		using EventCallbackFn = std::function<void(Event&)>;
		struct GLFWData
		{
			GLFWwindow* window;
			int pWidth;
			int pHeight;
			std::string pTitle;
			bool framebufferResized = false;

			EventCallbackFn callback;
		};

	    void SetEventCallback(const EventCallbackFn& func){ pData.callback = func; }
	    EventCallbackFn callback;

	private:
		static Window * pInstance;
        void initveWindow();
        static void frameBufferResizedCallback(GLFWwindow *window, int width, int height);

        std::string pTitle;

		GLFWData pData;
    };
}
