#include "Core/Window.hpp"
#include "Core/Log.hpp"
#include "Events/KeyEvents.hpp"
#include "Events/MouseEvents.hpp"

namespace Nyxis
{
	Window::Window(int width, int height, const std::string &name)
    {
		m_Data.pWidth = width;
		m_Data.pHeight = height;

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Data.window = glfwCreateWindow(m_Data.pWidth, m_Data.pHeight, name.c_str(), nullptr, nullptr);
		glfwSetWindowSizeLimits(m_Data.window, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
		glfwSetWindowUserPointer(m_Data.window, this);
		glfwSetFramebufferSizeCallback(m_Data.window, frameBufferResizedCallback);

	    glfwSetWindowUserPointer(m_Data.window, &m_Data);

	    glfwSetErrorCallback([](int error, const char* description)
	    {
			LOG_ERROR("GLFW Error ({0}): {1}", error, description);
	    });

		glfwSetKeyCallback(m_Data.window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
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

	    glfwSetCursorPosCallback(m_Data.window, [](GLFWwindow* window,double xPos, double yPos)
		{
			GLFWData& data = *(GLFWData*)glfwGetWindowUserPointer(window);
			MouseMovedEvent event(xPos, yPos);
			data.callback(event);
		});

    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Data.window);
        glfwTerminate();
    }

	void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, m_Data.window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Window::frameBufferResizedCallback(GLFWwindow *window, int width, int height)
    {
        GLFWData* data = reinterpret_cast<GLFWData*>(glfwGetWindowUserPointer(window));
        data->framebufferResized = true;
        data->pWidth = width;
        data->pHeight = height;
    }

} // namespace Nyxis