#pragma once
#include "Events/Event.hpp"

namespace Nyxis
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) : m_MouseX{ x }, m_MouseY{ y } {}
		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)


		float m_MouseX;
		float m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset) : pXOffset{ xOffset }, pYOffset{ yOffset } {}
		float GetXOffset() const { return pXOffset; }
		float GetYOffset() const { return pYOffset; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << pXOffset << ", " << pYOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);

		float pXOffset;
		float pYOffset;
	};

	class MouseButtonPressed : public Event 
	{
	public:
		MouseButtonPressed(int button, int mod = 0) : m_Button{ button }, m_Mod { mod } {}
		int GetButton() const { return m_Button; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressed: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);

		int m_Mod;
		int m_Button;
	};

	class MouseButtonReleased : public Event 
	{
	public:
		MouseButtonReleased(int button) : m_Button{ button } {}
		inline int GetButton() const { return m_Button; }

		std::string toString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleased: " << m_Button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased);
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput);

		int m_Button;
	};
	enum MouseCodes : int
	{
		MouseButtonLeft = 0,
		MouseButtonRight = 1,
		MouseButtonMiddle = 2
	};

}