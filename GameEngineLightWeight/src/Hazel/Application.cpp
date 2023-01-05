#include "hzpch.h"
#include "Application.h"

#include "Hazel/Log.h"

#include "Hazel/Renderer/Renderer.h"

//#include <glad/glad.h>

#include "Input.h"

namespace Hazel {
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)
	Application* Application::s_Instance = nullptr;

	Application::Application() {
		//HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// ��������
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		// ����imgui
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		// �����Ѿ�������window��imguilayer
		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		// 1.������������
		m_VertexArray.reset(VertexArray::Create());

		// 2.�������㻺����
		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		// 2.1���ö��㻺��������
		BufferLayout layout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"}
		};
		vertexBuffer->SetLayout(layout);

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		// 3.��������
		uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// 4.��ɫ��
		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			void main(){
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = vec4(a_Position, 1.0);
			}			
		)";
		std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main(){
				color = vec4(v_Position * 0.5 + 0.5, 1.0);	
				color = v_Color;
			}			
		)";
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));

		// �������ݣ���Ⱦ������
		float squareVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		// 1.������������
		m_SquareVA.reset(VertexArray::Create());

		// 2.�������㻺����
		std::shared_ptr<VertexBuffer> squareVB;
		squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		// 2.1���ö��㻺��������
		squareVB->SetLayout({
			{ShaderDataType::Float3, "a_Position"}
		});

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_SquareVA->AddVertexBuffer(squareVB);

		// 3.��������
		uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };

		std::shared_ptr<IndexBuffer> squareIB;
		squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_SquareVA->SetIndexBuffer(squareIB);

		// 4.��ɫ��
		std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;

			void main(){
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
			}			
		)";
		std::string blueShaderfragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main(){
				color = vec4(0.2, 0.3, 0.8, 1.0);	
			}			
		)";
		m_BlueShader.reset(new Shader(blueShaderVertexSrc, blueShaderfragmentSrc));
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
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));// OnWindowClose�Ǳ���ķ���������ȡ����Ϸѭ����

		//HZ_CORE_TRACE("{0}", e);

		// �Ӻ���ǰ
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}
	void Application::Run() {
		while (m_Running) {
			//glClearColor(0.1f, 0.1f, 0.1f, 1);
			//glClear(GL_COLOR_BUFFER_BIT);
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();

			Renderer::BeginScene();

			// ������
			m_BlueShader->Bind();		// ��shader
			Renderer::Submit(m_SquareVA);
			//m_SquareVA->Bind();		// �󶨶�������
			//glDrawElements(GL_TRIANGLES, m_SquareVA->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);


			// ������
			m_Shader->Bind();		// ��shader
			Renderer::Submit(m_VertexArray);
			//m_VertexArray->Bind();	// �󶨶�������
			//glDrawElements(GL_TRIANGLES, m_VertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);

			Renderer::EndScene();

			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate();
			}
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImgGuiRender();
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) {
		m_Running = false;
		return true;
	}
}
