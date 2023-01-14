#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Hazel/Renderer/Renderer2D.h>

#include <chrono>
#include <string>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	//Hazel::Renderer2D::Init();
	m_SquareTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
}

void Sandbox2D::OnDetach()
{
	HZ_PROFILE_FUNCTION();

}

Sandbox2D::~Sandbox2D()
{
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
	HZ_PROFILE_FUNCTION();

	m_CameraController.OnUpdate(ts);
	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::Clear();
	}
	{
		HZ_PROFILE_SCOPE("Renderer Draw");

		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Hazel::Renderer2D::DrawrRotatedQuad({ 1.0f, 0.5f }, { 0.8f, 0.8f },30.0f, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.8f }, {0.2f, 0.8f, 0.9f, 1.0f});
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_SquareTexture, 10.0f);
		Hazel::Renderer2D::DrawrRotatedQuad({ -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f }, rotation, m_SquareTexture, 20.0f);
		Hazel::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImgGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_FlatColor));

	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	// ÊÂ¼þ
	m_CameraController.OnEvent(event);
}