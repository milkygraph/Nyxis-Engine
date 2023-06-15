#include "Core/Nyxis.hpp"
#include "Core/Input.hpp"
#include "Core/Window.hpp"

namespace Nyxis
{
    Input* Input::pInstance = new Input();
	int Input::CursorMode = 0;

	bool Input::IsKeyPressedImpl(int key)
	{
		auto state = glfwGetKey(Window::GetGLFWwindow(), key);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	
	bool Input::IsMouseButtonPressedImpl(int button)
	{
		auto state = glfwGetMouseButton(Window::GetGLFWwindow(), button);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonReleasedImpl(int button)
	{
		return glfwGetMouseButton(Window::GetGLFWwindow(), button) == GLFW_RELEASE;
	}

	
	glm::vec2 Input::GetMousePositionImpl()
	{
		double xpos, ypos;
		glfwGetCursorPos(Window::GetGLFWwindow(), &xpos, &ypos);
		return { xpos, ypos };
	}

	void Input::SetCursorModeImpl(int mode)
	{
		glfwSetInputMode(Window::GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL + mode);
		CursorMode = mode;
	}

	// float Input::getMouseXImpl()
	// {
	// 	auto [x, y] = GetMousePositionImpl();
	// 	return x;
	// }

	// float Input::getMouseYImpl()
	// {
	// 	auto [x, y] = GetMousePositionImpl();
	// 	return y;
	// }
}