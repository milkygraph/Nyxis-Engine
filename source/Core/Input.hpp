#pragma once
#include "Core/Nyxispch.hpp"
#include "Events/KeyEvents.hpp"

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

        static bool IsKeyPressed(int key, int mod = 0)
        {
			switch(mod)
			{
			case 1:
				return IsKeyPressedImpl(key) && IsKeyPressedImpl(KeyCodes::LeftShift);
			case 4:
				return IsKeyPressedImpl(key) && IsKeyPressedImpl(KeyCodes::LeftControl);
			case 8:
				return IsKeyPressedImpl(key) && IsKeyPressedImpl(KeyCodes::LeftAlt);
			default:
				return IsKeyPressedImpl(key);
			}
        }

        static bool IsMouseButtonPressed(int button) { return pInstance->IsMouseButtonPressedImpl(button); }
        static bool IsMouseButtonReleased(int button) { return pInstance->IsMouseButtonReleasedImpl(button); }
        static glm::vec2 GetMousePosition() { return pInstance->GetMousePositionImpl(); }
        static float GetMouseX() { return pInstance->GetMousePositionImpl().x; }
        static float GetMouseY() { return pInstance->GetMousePositionImpl().y; }
		static CursorMode GetCursorMode() { return pInstance->m_CursorMode; }
		static void SetCursorMode(int mode) { pInstance->SetCursorModeImpl(mode); }

    private:
        static Input* pInstance;

		static inline CursorMode m_CursorMode = CursorNormal;

		// bits representing the state of the mouse buttons such as released, pressed, held or double clicked
		struct MouseButtonState
		{
			bool released = false;
			bool pressed = false;
			bool held = false;
			bool doubleClicked = false;
		};

		// 8 buttons in total (GLFW_MOUSE_BUTTON_LAST)
		static MouseButtonState m_MouseButtonStates[8];

		static bool IsMouseButtonReleasedImpl(int button);
        static bool IsKeyPressedImpl(int key);
        static bool IsMouseButtonPressedImpl(int button);
        static glm::vec2 GetMousePositionImpl();
		static void SetCursorModeImpl(int mode);
	};
}
