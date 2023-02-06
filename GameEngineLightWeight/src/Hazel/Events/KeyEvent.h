#pragma once

#include "Event.h"
#include "Hazel/Core/KeyCodes.h"


namespace Hazel {
	class HAZEL_API KeyEvent :public Event {
	public:
		inline KeyCode GetKeyCode()const { return m_KeyCode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(KeyCode keycode) : m_KeyCode(keycode){
		}
		
		KeyCode m_KeyCode;
	};
	
	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
			: KeyEvent(keycode), m_IsRepeat(isRepeat) {}

		bool IsRepeat() const { return m_IsRepeat; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_IsRepeat;
	};

	class HAZEL_API KeyReleaseEvent :public KeyEvent {
	public:
		KeyReleaseEvent(KeyCode keycode) :KeyEvent(keycode) {}
		std::string ToString()const override {
			std::stringstream ss;
			ss << "KeyReleaseEvent:" << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased);
	};

	class HAZEL_API KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(KeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString()const override {
			std::stringstream ss;
			ss << "KeyTypedEvent:" << m_KeyCode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyTyped)
	};
}