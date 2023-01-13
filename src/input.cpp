#include "ve.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>

namespace ve
{
    Input* Input::pInstance = new Input();

	bool Input::isKeyPressedImpl(GLFWwindow* window, int key)
	{
		auto state = glfwGetKey(window, key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::isMouseButtonPressedImpl(GLFWwindow* window, int button)
	{
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}
	
	std::pair<float, float> Input::getMousePositionImpl(GLFWwindow* window)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}
	
	float Input::getMouseXImpl(GLFWwindow* window)
	{
		auto [x, y] = getMousePositionImpl(window);
		return x;
	}

	float Input::getMouseYImpl(GLFWwindow* window)
	{
		auto [x, y] = getMousePositionImpl(window);
		return y;
	}
}