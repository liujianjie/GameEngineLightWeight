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
	class HAZEL_API Application
	{
	public:
		Application(const std::string& name = "Game Engine", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
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

		ApplicationCommandLineArgs GetCommandLineArgs()const { return m_CommandLineArgs; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

		// ����deltatime,ÿһ֡�ļ��ʱ��
		float m_LastFrameTime = 0.0f;
	private:
		ApplicationCommandLineArgs m_CommandLineArgs;
		static Application* s_Instance;
	};

	// ���ڿͻ��˱�����
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

