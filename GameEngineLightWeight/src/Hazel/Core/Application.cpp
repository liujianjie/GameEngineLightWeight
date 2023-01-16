#include "hzpch.h"
#include "Application.h"

#include "Hazel/Core/Log.h"

#include "Hazel/Renderer/Renderer.h"

#include "Input.h"

#include <GLFW/glfw3.h>

namespace Hazel {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
	Application* Application::s_Instance = nullptr;

	/*
		���ڴ��ڵĴ�С��1280 ��720����16 / 9 = 1.77777
		��ô����m_Camera��left ���� -1.6,bottomΪ-0.9�Ϳ��Խ������
		����ô�о����ˣ�����
		1280 * 0.9 = 720 * 1.6,��ôleft��-1.6��������0.9...


		:m_Camera(-2.0f, 2.0f, -2.0f, 2.0f)
		:m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
		:m_Camera(-1.0f, 1.0f, -1.0f, 1.0f)
	*/
	Application::Application(const std::string& name)
	{
		HZ_PROFILE_FUNCTION();
		//HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// ��������
		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
		//m_Window->SetVSync(true);
		//m_Window->SetVSync(false);

		// ����imgui
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// �����Ѿ�������window��imguilayer

		// ��ʼ����Ⱦ
		Renderer::Init();
	}
	Application::~Application() {
		HZ_PROFILE_FUNCTION();
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
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));// OnWindowClose�Ǳ���ķ���������ȡ����Ϸѭ����
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
		//HZ_CORE_TRACE("{0}", e);

		// �Ӻ���ǰ
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}
	void Application::Run() {
		HZ_PROFILE_FUNCTION();

		while (m_Running) {
			HZ_PROFILE_SCOPE("RunLoop");
			// �ó�ÿһ֡�ļ��ʱ��
			float time = (float)glfwGetTime(); // �Ǵ�Ӧ�ÿ�ʼ�����ܹ���ʱ��
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized) {
				{
					HZ_PROFILE_SCOPE("LayerStack OnUpdate");
					// ÿһ����update
					for (Layer* layer : m_LayerStack) {
						layer->OnUpdate(timestep);
					}
				}
			}
			// imgui��update
			m_ImGuiLayer->Begin();
			{
				HZ_PROFILE_SCOPE("LayerStack OnImGuiRender");
				for (Layer* layer : m_LayerStack)
					layer->OnImgGuiRender();
				m_ImGuiLayer->End();

			}
			// ������update
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
