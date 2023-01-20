#pragma once
#include "Events/event.hpp"


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <string>

namespace ve
{
    class veWindow
    {
    public:
	    veWindow(int width, int height, const std::string &name);
        ~veWindow();

        veWindow(const veWindow &) = delete;
        veWindow &operator=(const veWindow &) = delete;

        VkExtent2D getExtent() { return {static_cast<uint32_t>(pData.pWidth), static_cast<uint32_t>(pData.pHeight)}; }
        bool shouldClose() { return glfwWindowShouldClose(pData.window); }
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        bool windowResized() { return pData.framebufferResized; }
        void resetWindowResizedFlag() { pData.framebufferResized = false; };

        static GLFWwindow* getGLFWwindow() { return pInstance->pData.window; }
		static veWindow& get(int width, int height, const std::string& name)
		{
			pInstance = new veWindow(width, height, name);
			return *pInstance;
		}

		static veWindow& get() { return *pInstance; }


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
		static veWindow* pInstance;
        void initveWindow();
        static void frameBufferResizedCallback(GLFWwindow *window, int width, int height);

        std::string pTitle;

		GLFWData pData;
    };
}
