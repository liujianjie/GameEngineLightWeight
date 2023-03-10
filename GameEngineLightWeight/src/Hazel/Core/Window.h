#pragma once
#include "hzpch.h"

#include "Hazel/Core/Core.h"
#include "Hazel/Events/Event.h"

namespace Hazel {
	struct WindowProps {
		std::string Title;
		unsigned int Width;
		unsigned int Height;
		WindowProps(const std::string& title = "Game Engine",
			unsigned int width = 1600,
			unsigned int height = 900) :Title(title), Width(width), Height(height)
		{
		}
	};

	class HAZEL_API Window {
	public:
		using EventCallbackFn = std::function<void(Event&)>;// 返回值是void的，参数是Event的调用形式

		virtual ~Window(){}
		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindow() const = 0;

		static Scope<Window> Create(const WindowProps& props = WindowProps());
	};
}