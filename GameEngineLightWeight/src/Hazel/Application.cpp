#include "hzpch.h"
#include "Application.h"

#include "Hazel/Log.h"

#include "Hazel/Renderer/Renderer.h"

#include "Input.h"

namespace Hazel {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
	Application* Application::s_Instance = nullptr;

	/*
		由于窗口的大小是1280 ：720，是16 / 9 = 1.77777
		那么设置m_Camera的left 设置 -1.6,bottom为-0.9就可以解决？？
		我怎么感觉反了，明明
		1280 * 0.9 = 720 * 1.6,怎么left是-1.6，而不是0.9...


		:m_Camera(-2.0f, 2.0f, -2.0f, 2.0f)
		:m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
		:m_Camera(-1.0f, 1.0f, -1.0f, 1.0f)
	*/
	Application::Application()
	{
		//HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// 创建窗口
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		// 创建imgui
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// 上面已经创建了window和imguilayer
		
	}
	Application::~Application() {

	}
	void Application::PushLayer(Layer* layer) {
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer) {
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}
	void Application::OnEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));// OnWindowClose是本类的方法，用来取消游戏循环的

		//HZ_CORE_TRACE("{0}", e);

		// 从后往前
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}
	void Application::Run() {
		while (m_Running) {
			// 每一层在update
			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate();
			}
			// imgui在update
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImgGuiRender();
			m_ImGuiLayer->End();
			// 窗口在update
			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}
