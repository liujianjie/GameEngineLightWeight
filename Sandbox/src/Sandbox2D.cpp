#include "Sandbox2D.h"
#include "imgui/imgui.h"
#include <Platform/OpenGL/OpenGLShader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	// 渲染网格 flat
	float flatVertices[3 * 4] = {
		-0.75f, -0.75f, 0.0f,
		0.75f, -0.75f, 0.0f,
		0.75f,  0.75f, 0.0f,
		-0.75f,  0.75f, 0.0f
	};
	// 1.创建顶点数组
	m_FlatVertexArray = (Hazel::VertexArray::Create());

	// 2.创建顶点缓冲区
	Hazel::Ref<Hazel::VertexBuffer> flatVB;
	flatVB.reset(Hazel::VertexBuffer::Create(flatVertices, sizeof(flatVertices)));

	// 2.1设置顶点缓冲区布局
	flatVB->SetLayout({
		{Hazel::ShaderDataType::Float3, "a_Position"}
		});

	// 1.1顶点数组添加顶点缓冲区，并且在这个缓冲区中设置布局
	m_FlatVertexArray->AddVertexBuffer(flatVB);

	// 3.索引缓冲
	uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };

	Hazel::Ref<Hazel::IndexBuffer> flatIB;
	flatIB.reset(Hazel::IndexBuffer::Create(flatIndices, sizeof(flatIndices) / sizeof(uint32_t)));

	// 1.2顶点数组设置索引缓冲区
	m_FlatVertexArray->SetIndexBuffer(flatIB);

	m_FlatShader = (Hazel::Shader::Create("assets/shaders/FlatColor.glsl"));
}

void Sandbox2D::OnDetach()
{
}

Sandbox2D::~Sandbox2D()
{
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Hazel::RenderCommand::Clear();

	Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

	std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->Bind();
	std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->UploadUniformFloat4("u_Color", m_FlatColor);
	glm::mat4 squareTexCoordtransfrom = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f });
	Hazel::Renderer::Submit(m_FlatShader, m_FlatVertexArray, squareTexCoordtransfrom);

	Hazel::Renderer::EndScene();
}

void Sandbox2D::OnImgGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_FlatColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	// 事件
	m_CameraController.OnEvent(event);
}