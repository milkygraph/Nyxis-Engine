#pragma once

#include "ve.hpp"

namespace ve
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, // I am not sure if we have to keep these since we already have them implemented
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum class EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType(){ return EventType::##type; } \
							   virtual EventType GetEventType() const override { return GetStaticType(); } \
							   virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; } 
	
	class Event
	{
	public:
		bool mHandled = false;

		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual EventCategory getCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }
		
		inline bool isHandled() const { return mHandled; }
		inline bool isSubCategory(const EventCategory category) const 
		{
			// compare the category flags with the category
			// fix this
			return getCategoryFlags() == category;
		}
	};

	class EventDispatcher
	{
	public:
		template<typename T> using EventFn = std::function<bool(T&)>;
		EventDispatcher(Event& event) : pEvent(event) {}
		template<typename T> bool dispatch(EventFn<T> func)
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