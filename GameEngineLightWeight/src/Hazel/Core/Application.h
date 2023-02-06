#pragma once

#include "Base.h"

#include "Window.h"
#include "Hazel/Core/LayerStack.h"

#include "Hazel/Events/Event.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Hazel/ImGui/ImGuiLayer.h"

#include "Hazel/Core/Timestep.h"

namespace Hazel {
	struct ApplicationCommandLineArgs {
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			HZ_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};
	struct ApplicationSpecification {
		std::string Name = "Game Engine Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};
	class HAZEL_API Application
	{
	public:
		//Application(const std::string& name = "Game Engine", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		//inline OrthographicCamera& GetCamera() { return m_Camera; }
		void Close();

		const ApplicationSpecification GetSpecification()const { return m_Specification; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

		// 计算deltatime,每一帧的间隔时间
		float m_LastFrameTime = 0.0f;
	private:
		ApplicationSpecification m_Specification;
		//ApplicationCommandLineArgs m_CommandLineArgs;
		static Application* s_Instance;
	};

	// 将在客户端被定义
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

