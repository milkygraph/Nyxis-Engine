#include "window.hpp"
#include "Events/keyEvents.hpp"
#include "Events/mouseEvents.hpp"
#include "Log.hpp"

#include <stdexcept>

namespace ve
{
	veWindow* veWindow::pInstance = nullptr;
    veWindow::veWindow(int width, int height, const std::string &name) : pTitle(name)
    {
		pData.pWidth = width;
		pData.pHeight = height;

        initveWindow();

	    glfwSetWindowUserPointer(pData.window, &pData);

	    glfwSetErrorCallback([](int error, const char* description)
	    {
			LOG_ERROR("GLFW Error ({0}): {1}", error, description);
	    });

		glfwSetKeyCallback(pData.window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	    {
		    GLFWData& data = *(GLFWData*)glfwGetWindowUserPointer(window);
		    switch (action)
		    {
		    case GLFW_PRESS:
		    {
			    KeyPressedEvent event(key, 0);
			    data.callback(event);
			    break;
		    }
		    case GLFW_RELEASE:
		    {
			    KeyReleasedEvent event(key);
			    data.callback(event);
			    break;
		    }
		    case GLFW_REPEAT:
		    {
			    KeyPressedEvent event(key, 1);
			    data.callback(event);
			    break;
		    }
		    }
	    });

	    glfwSetCursorPosCallback(pData.window, [](GLFWwindow* window,double xPos, double yPos)
		{
			GLFWData& data = *(GLFWData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event(xPos, yPos);
			data.callback(event);
		});

    }

    veWindow::~veWindow()
    {
        glfwDestroyWindow(pData.window);
        glfwTerminate();
    }

    void veWindow::initveWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        pData.window = glfwCreateWindow(pData.pWidth, pData.pHeight, pTitle.c_str(), nullptr, nullptr);
        glfwSetWindowSizeLimits(pData.window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowUserPointer(pData.window, this);
        glfwSetFramebufferSizeCallback(pData.window, frameBufferResizedCallback);
    }

    void veWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, pData.window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void veWindow::frameBufferResizedCallback(GLFWwindow *window, int width, int height)
    {
        GLFWData* data = reinterpret_cast<GLFWData*>(glfwGetWindowUserPointer(window));
        data->framebufferResized = true;
        data->pWidth = width;
        data->pHeight = height;
    }

} // namespace ve