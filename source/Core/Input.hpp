#pragma once
#include "Core/Nyxispch.hpp"

namespace Nyxis
{
	enum CursorMode : int
	{
		CursorNormal = 0,
		CursorHidden = 1,
		CursorDisabled = 2
	};

	class Input
	{
	public:
        Input() = default;
        ~Input() = default;

        static bool isKeyPressed(int key) { return pInstance->isKeyPressedImpl(key); };
        static bool isMouseButtonPressed(int button) { return pInstance->isMouseButtonPressedImpl(button); };
        static glm::vec2 getMousePosition() { return pInstance->getMousePositionImpl(); };
        float getMouseX() { return pInstance->getMousePositionImpl().x; }
        float getMouseY() { return pInstance->getMousePositionImpl().y; }
		static void setCursorMode(int mode) { pInstance->setCursorModeImpl(mode); }
		static int getCursorMode() { return pInstance->CursorMode; }

    private:
        static Input* pInstance;
		static int CursorMode;

        static bool isKeyPressedImpl(int key);
        static bool isMouseButtonPressedImpl(int button);
        static glm::vec2 getMousePositionImpl();
		static void setCursorModeImpl(int mode);
	};
}