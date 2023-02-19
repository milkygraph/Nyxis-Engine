#pragma once

#include "ve.hpp"
#include "vepch.hpp"

namespace Nyxis
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, // I am not sure if we have to keep these since we already have them implemented
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType getStaticType(){ return EventType::type; } \
							   virtual EventType getEventType() const override { return getStaticType(); } \
							   virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }
	
	class Event
	{
	public:
		bool mHandled = false;

		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }
		
		inline bool isHandled() const { return mHandled; }
		inline bool isSubCategory(const EventCategory category) const 
		{
			return getCategoryFlags() & category;
		}
	};

	class EventDispatcher
	{
	public:
		template<typename T>
		EventDispatcher(Event& event) : pEvent(event) {}

		template<typename T, typename F> bool dispatch(const F& func)
		{
			if (pEvent.getEventType() == T::getStaticType())
			{
				pEvent.mHandled = func(*(T*)&pEvent);
				return true;
			}
			return false;
		}

	private:
		Event& pEvent;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.toString();
	}
}