#include "ve.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>
#include "window.hpp"

namespace ve
{
    Input* Input::pInstance = new Input();

	bool Input::isKeyPressedImpl(int key)
	{
		auto state = glfwGetKey(veWindow::getGLFWwindow(), key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::isMouseButtonPressedImpl(int button)
	{
		auto state = glfwGetMouseButton(veWindow::getGLFWwindow(), button);
		return state == GLFW_PRESS;
	}
	
	std::pair<float, float> Input::getMousePositionImpl()
	{
		double xpos, ypos;
		glfwGetCursorPos(veWindow::getGLFWwindow(), &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}
	
	// float Input::getMouseXImpl()
	// {
	// 	auto [x, y] = getMousePositionImpl();
	// 	return x;
	// }

	// float Input::getMouseYImpl()
	// {
	// 	auto [x, y] = getMousePositionImpl();
	// 	return y;
	// }
}