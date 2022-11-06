#include <window.hpp>

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

        pWindow = glfwCreateWindow(pWidth, pHeight, pTitle.c_str(), nullptr, nullptr);
    }

    void veWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, pWindow, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
} // namespace ve