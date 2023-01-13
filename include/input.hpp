#pragma once

#include "ve.hpp"
#include "Events/event.hpp"

#include <GLFW/glfw3.h>

namespace ve
{
	class Input
	{
	public:
        Input() = default;
        ~Input() = default;

        static bool isKeyPressed(GLFWwindow* window, int key) { return pInstance->isKeyPressedImpl(window, key); };
        static bool isMouseButtonPressed(GLFWwindow* window, int button) { return pInstance->isMouseButtonPressedImpl(window, button); };
        static std::pair<float, float> getMousePosition(GLFWwindow* window) { return pInstance->getMousePositionImpl(window); };
        float getMouseX(GLFWwindow* window) { return pInstance->getMousePositionImpl(window).first; }
        float getMouseY(GLFWwindow* window) { return pInstance->getMousePositionImpl(window).second; }

    private:
        static Input* pInstance;

        static bool isKeyPressedImpl(GLFWwindow* window, int key);
        static bool isMouseButtonPressedImpl(GLFWwindow* window, int button);
        static std::pair<float, float> getMousePositionImpl(GLFWwindow* window);
        float getMouseXImpl(GLFWwindow* window);
        float getMouseYImpl(GLFWwindow* window);
    };
}
