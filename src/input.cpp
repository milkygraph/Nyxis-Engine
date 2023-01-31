#include "ve.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>
#include "window.hpp"

namespace ve
{
    Input* Input::pInstance = new Input();
	int Input::CursorMode = 0;

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
	
	glm::vec2 Input::getMousePositionImpl()
	{
		double xpos, ypos;
		glfwGetCursorPos(veWindow::getGLFWwindow(), &xpos, &ypos);
		return { xpos, ypos };
	}

	void Input::setCursorModeImpl(int mode)
	{
		glfwSetInputMode(veWindow::getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL + mode);
		CursorMode = mode;
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