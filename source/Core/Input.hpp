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
        float GetMouseX() { return pInstance->GetMousePositionImpl().x; }
        float GetMouseY() { return pInstance->GetMousePositionImpl().y; }
		static void SetCursorMode(int mode) { pInstance->SetCursorModeImpl(mode); }
		static int GetCursorMode() { return pInstance->CursorMode; }

    private:
        static Input* pInstance;
		static int CursorMode;

		static bool IsMouseButtonReleasedImpl(int button);
        static bool IsKeyPressedImpl(int key);
        static bool IsMouseButtonPressedImpl(int button);
        static glm::vec2 GetMousePositionImpl();
		static void SetCursorModeImpl(int mode);
	};
}
