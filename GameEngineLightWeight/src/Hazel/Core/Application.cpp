#include "hzpch.h"
#include "Application.h"

#include "Hazel/Core/Log.h"

#include "Hazel/Renderer/Renderer.h"

#include "Input.h"

#include <GLFW/glfw3.h>
#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Scripting/ScriptEngine.h"

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
	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		HZ_PROFILE_FUNCTION();
		//HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// 设置工作目录
		if (!m_Specification.WorkingDirectory.empty()) {
			std::filesystem::current_path(m_Specification.WorkingDirectory);
		}
		// 创建窗口
		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		//m_Window->SetVSync(true);
		//m_Window->SetVSync(false);

		// 创建imgui
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// 上面已经创建了window和imguilayer

		// 初始化渲染
		Renderer::Init();
		ScriptEngine::Init();
	}
	Application::~Application() {
		HZ_PROFILE_FUNCTION();

		//ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}
	void Application::PushLayer(Layer* layer) {
		HZ_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer) {
		HZ_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}
	void Application::OnEvent(Event& e) {
		HZ_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));// OnWindowClose是本类的方法，用来取消游戏循环的
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
		//HZ_CORE_TRACE("{0}", e);

		// 从后往前
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			if (e.Handled)
				break;
			(*--it)->OnEvent(e);
		}
	}
	void Application::Run() {
		HZ_PROFILE_FUNCTION();

		while (m_Running) {
			HZ_PROFILE_SCOPE("RunLoop");
			// 得出每一帧的间隔时间
			//float time = (float)glfwGetTime(); // 是从应用开始计算总共的时间
			float time = Time::GetTime(); // 是从应用开始计算总共的时间
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized) {
				{
					HZ_PROFILE_SCOPE("LayerStack OnUpdate");
					// 每一层在update
					for (Layer* layer : m_LayerStack) {
						layer->OnUpdate(timestep);
					}
				}
			}
			// imgui在update
			m_ImGuiLayer->Begin();
			{
				HZ_PROFILE_SCOPE("LayerStack OnImGuiRender");
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();

			}
			// 窗口在update
			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		HZ_PROFILE_FUNCTION();
		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}
	void Application::Close() {
		m_Running = false;
	}
}
