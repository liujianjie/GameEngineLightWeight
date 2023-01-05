#pragma once

#include "Core.h"

#include "Window.h"
#include "Hazel/LayerStack.h"

#include "Events/Event.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Renderer/Shader.h"

#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/Buffer.h"

#include "Hazel/Renderer/VertexArray.h"

namespace Hazel {
	class HAZEL_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		//unsigned int m_VertexArray;
		//std::unique_ptr<Shader> m_Shader;
		//std::unique_ptr<VertexBuffer> m_VertexBuffer;
		//std::unique_ptr<IndexBuffer> m_IndexBuffer;

		std::shared_ptr<Shader> m_Shader;				// shader类 指针
		std::shared_ptr<VertexArray> m_VertexArray;		// 顶点数组类 指针

		std::shared_ptr<Shader> m_BlueShader;			// shader类 指针
		std::shared_ptr<VertexArray> m_SquareVA;			// 顶点数组类 指针
	private:
		static Application* s_Instance;
	};

	// 将在客户端被定义
	Application* CreateApplication();
}

