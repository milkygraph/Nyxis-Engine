#include "window.hpp"

#include <stdexcept>

namespace ve
{
    veWindow::veWindow(int width, int height, const std::string &name) : pWidth(width), pHeight(height), pTitle(name)
    {
        initveWindow();
    }

    veWindow::~veWindow()
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }

    void veWindow::initveWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        // glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        pWindow = glfwCreateWindow(pWidth, pHeight, pTitle.c_str(), nullptr, nullptr);
        glfwSetWindowSizeLimits(pWindow, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowUserPointer(pWindow, this);
        glfwSetFramebufferSizeCallback(pWindow, frameBufferResizedCallback);
    }

    void veWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, pWindow, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void veWindow::frameBufferResizedCallback(GLFWwindow *window, int width, int height)
    {
        auto pWindow = reinterpret_cast<veWindow *>(glfwGetWindowUserPointer(window));
        pWindow->framebufferResized = true;
        pWindow->pWidth = width;
        pWindow->pHeight = height;
    }

} // namespace ve