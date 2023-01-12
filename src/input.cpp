
#include "ve.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>

namespace ve
{
	bool Input::isKeyPressed(int keycode)
	{
		auto state = glfwGetKey(pWindow, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::isMouseButtonPressed(int button)
	{
		auto state = glfwGetMouseButton(pWindow, button);
		return state == GLFW_PRESS;
	}
	
	std::pair<float, float> Input::getMousePosition()
	{
		double xpos, ypos;
		glfwGetCursorPos(pWindow, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}
	
	float Input::getMouseXImpl()
	{
		auto [x, y] = getMousePosition();
		return x;
	}

	float Input::getMouseYImpl()
	{
		auto [x, y] = getMousePosition();
		return y;
	}
}