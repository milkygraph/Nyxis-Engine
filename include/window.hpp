#pragma once

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

        VkExtent2D getExtent() { return {static_cast<uint32_t>(pWidth), static_cast<uint32_t>(pHeight)}; }
        bool shouldClose() { return glfwWindowShouldClose(pWindow); }
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        bool windowResized() { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; };

        static GLFWwindow* getGLFWwindow() { return pInstance->pWindow; }
		static veWindow& get(int width, int height, const std::string& name)
		{
			pInstance = new veWindow(width, height, name);
			return *pInstance;
		}

		static veWindow& get() { return *pInstance; }


    private:
		static veWindow* pInstance;
        void initveWindow();
        static void frameBufferResizedCallback(GLFWwindow *window, int width, int height);

		GLFWwindow* pWindow;

        int pWidth;
        int pHeight;
        bool framebufferResized = false;

        std::string pTitle;
    };
}