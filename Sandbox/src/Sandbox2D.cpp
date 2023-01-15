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
	// ����Texture sheet
	m_SpriteSheet = Hazel::Texture2D::Create("assets/games/textures/RPGpack_sheet_2X.png");
	//m_TextureStairs, m_TextureTree, m_TextureBush
	m_TextureStair = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 }, {128, 128});
	m_TextureBush = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 3 }, { 128, 128 });
	m_TextureTree = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 4, 1 }, { 128, 128 }, { 1,2 });
	// Init here
	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 1.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };
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

	// ��Ⱦ��Ϣ��ʼ��
	Hazel::Renderer2D::ResetStats();

	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::Clear();
	}
	{
#if 0
		HZ_PROFILE_SCOPE("Renderer Draw");

		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Hazel::Renderer2D::DrawrRotatedQuad({ 1.0f, 0.5f }, { 0.8f, 0.8f },30.0f, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.8f }, {0.2f, 0.8f, 0.9f, 1.0f});
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_SquareTexture, 10.0f);
		Hazel::Renderer2D::DrawrRotatedQuad({ -0.5f, -1.5f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_SquareTexture, 20.0f);
		Hazel::Renderer2D::EndScene();

		// �����µĻ��ƣ������û����ڴ�
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f) {
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f , (y + 5.0f) /10.0f , 0.7f };
				Hazel::Renderer2D::DrawQuad({ x, y }, {0.45f, 0.45f}, color);
			}
		}
		Hazel::Renderer2D::EndScene();
#endif
	}

	// ���λ��ת��������ռ� ��������
	if (Hazel::Input::IsMouseButtonPressed(HZ_MOUSE_BUTTON_LEFT))
	{
		auto [x, y] = Hazel::Input::GetMousePosition();
		auto width = Hazel::Application::Get().GetWindow().GetWidth();
		auto height = Hazel::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto pos = m_CameraController.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + pos.x, y + pos.y };
		for (int i = 0; i < 5; i++)
			m_ParticleSystem.Emit(m_Particle);
	}
	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());

	// ����������һ��
	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
	Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_TextureBush, 1.0f);
	Hazel::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_TextureStair, 1.0f);
	Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f, 0.9f }, { 1.0f, 2.0f }, m_TextureTree, 1.0f);
	Hazel::Renderer2D::EndScene();
}

void Sandbox2D::OnImgGuiRender()
{
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = Hazel::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_FlatColor));
	ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& event)
{
	// �¼�
	m_CameraController.OnEvent(event);
}