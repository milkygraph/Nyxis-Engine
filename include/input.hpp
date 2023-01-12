#pragma once

#include "ve.hpp"
#include "Events/event.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
	class Input
	{
	public:
		explicit Input(GLFWwindow* window) : pWindow(window) {}
		~Input() = default;
		bool isKeyPressed(int keycode);
		bool isMouseButtonPressed(int button);
		std::pair<float, float> getMousePosition();
		float getMouseXImpl();
		float getMouseYImpl();
	private:
		GLFWwindow* pWindow;
	};
}
