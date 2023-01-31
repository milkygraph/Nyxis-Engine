#pragma once

#include "ve.hpp"
#include "Events/event.hpp"

namespace ve
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) : pMouseX{ x }, pMouseY{ y } {}
		inline float GetX() const { return pMouseX; }
		inline float GetY() const { return pMouseY; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << pMouseX << ", " << pMouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float pMouseX;
		float pMouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset) : pXOffset{ xOffset }, pYOffset{ yOffset } {}
		inline float GetXOffset() const { return pXOffset; }
		inline float GetYOffset() const { return pYOffset; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << pXOffset << ", " << pYOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);

	private:
		float pXOffset;
		float pYOffset;
	};

	class MouseButtonPressed : public Event 
	{
	public:
		MouseButtonPressed(int button) : pButton{ button } {}
		inline int GetButton() const { return pButton; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressed: " << pButton;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);
	private:
		int pButton;
	};

	enum MouseButton : int
	{
		MouseButtonLeft = 0,
		MouseButtonRight = 1,
		MouseButtonMiddle = 2
	};
}