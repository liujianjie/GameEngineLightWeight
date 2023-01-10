#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Hazel/Renderer/Renderer2D.h>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	//Hazel::Renderer2D::Init();
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

	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Hazel::Renderer2D::DrawQuad({0.0f, 0.0f}, {1.0f,1.0f}, m_FlatColor);
	Hazel::Renderer2D::EndScene();
	//Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

	//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->Bind();
	//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->UploadUniformFloat4("u_Color", m_FlatColor);
	//glm::mat4 squareTexCoordtransfrom = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f });
	//Hazel::Renderer::Submit(m_FlatShader, m_FlatVertexArray, squareTexCoordtransfrom);

	//Hazel::Renderer::EndScene();
}

void Sandbox2D::OnImgGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_FlatColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	// ÊÂ¼þ
	m_CameraController.OnEvent(event);
}