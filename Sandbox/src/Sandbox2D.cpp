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
	m_SquareTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
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
		Hazel::Renderer2D::DrawQuad({-1.0f, 0.0f}, {0.8f,0.8f}, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.8f }, {0.2f, 0.8f, 0.9f, 1.0f});
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_SquareTexture);
		Hazel::Renderer2D::EndScene();
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