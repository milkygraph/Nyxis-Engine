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

        static bool isKeyPressed(int key) { return pInstance->isKeyPressedImpl(key); };
        static bool isMouseButtonPressed(int button) { return pInstance->isMouseButtonPressedImpl(button); };
        static std::pair<float, float> getMousePosition() { return pInstance->getMousePositionImpl(); };
        float getMouseX() { return pInstance->getMousePositionImpl().first; }
        float getMouseY() { return pInstance->getMousePositionImpl().second; }

    private:
        static Input* pInstance;

        static bool isKeyPressedImpl(int key);
        static bool isMouseButtonPressedImpl(int button);
        static std::pair<float, float> getMousePositionImpl();
    };
}
