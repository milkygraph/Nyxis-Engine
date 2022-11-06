#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

        bool shouldClose(){ return glfwWindowShouldClose(pWindow); }
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        GLFWwindow *pWindow;
        const int pWidth;
        const int pHeight;
        std::string pTitle;
        bool framebufferResized_ = false;
        void initveWindow();
    };
}