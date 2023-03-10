#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Hazel/Renderer/Renderer2D.h>

#include <chrono>
#include <string>

#if 0
/*
0不绘制
1是墙壁
2是草地
3是箱子
*/
static const uint32_t s_MapWidth = 20;
static const char* s_MapTiles =
"00000000000000000000"
"01111111111111111100"
"01222312332312323100"
"01213111121111213100"
"01212222222222212100"
"01211211121112113100"
"01213233333332213100"
"01331212313212133100"
"01111232111232311100"
"01113212313212132100"
"01311233333332112100"
"01321212121312333100"
"01313211121112113100"
"01311222222222212100"
"01211131121111312100"
"01213131121111212100"
"01223333233333322100"
"01111111111111111100"
"00000000000000000000"
"00000000000000000000"
;
#endif
Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f, true)
{
}

void Sandbox2D::OnAttach()
{
	HZ_PROFILE_FUNCTION();

	//Hazel::Renderer2D::Init();
	m_SquareTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png");
	// 加载Texture sheet
	m_SpriteSheet = Hazel::Texture2D::Create("assets/games/textures/RPGpack_sheet_2X.png");

	// 拉远摄像机
	m_CameraController.SetZoomLevel(5.0f);

	// Init here
	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 25.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

#if 0
	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / s_MapWidth;
	//m_TextureStairs, m_TextureTree, m_TextureBush
	m_TextureStair = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 6 }, {128, 128});
	m_TextureBush = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 3 }, { 128, 128 });
	m_TextureTree = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 4, 1 }, { 128, 128 }, { 1,2 });

	s_TextureMap['1'] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 10, 8 }, { 128, 128 });
	s_TextureMap['2'] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11 }, { 128, 128 });
	s_TextureMap['3'] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 11, 11 }, { 128, 128 });

	// 拉远摄像机
	m_CameraController.SetZoomLevel(8.0f);
#endif
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

	// 渲染信息初始化
	Hazel::Renderer2D::ResetStats();

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
		Hazel::Renderer2D::DrawRotatedQuad({ 1.0f, 0.5f }, { 0.8f, 0.8f }, glm::radians(30.0), m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.8f }, {0.2f, 0.8f, 0.9f, 1.0f});
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_SquareTexture, 10.0f);
		Hazel::Renderer2D::DrawRotatedQuad({ -0.5f, -1.5f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_SquareTexture, 20.0f);
		Hazel::Renderer2D::EndScene();

		// 开启新的绘制，会重置绘制内存
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f) {
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f , (y + 5.0f) /10.0f , 0.7f };
				Hazel::Renderer2D::DrawQuad({ x, y }, {0.45f, 0.45f}, color);
			}
		}
		Hazel::Renderer2D::EndScene();
	}


	// 鼠标位置转换成世界空间 绘制粒子
	if (Hazel::Input::IsMouseButtonPressed(Hazel::Mouse::ButtonLeft))
	{
		auto [x, y] = Hazel::Input::GetMousePosition();
		auto width = Hazel::Application::Get().GetWindow().GetWidth();
		auto height = Hazel::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto pos = m_CameraController.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetWidth() * 0.5f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + pos.x, y + pos.y };
		for (int i = 0; i < 500; i++)
			m_ParticleSystem.Emit(m_Particle);
	}
	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());

#if 0
	// 绘制纹理集的一个
	Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
	for (uint32_t y = 0; y < m_MapHeight; y++) {
		for (uint32_t x = 0; x < m_MapWidth; x++) {
			// x + y * m_MapWidth; 切割成2维数组
			char titleType = s_MapTiles[x + y * m_MapWidth];
			if (titleType == '0') { // 0的东西不画
				continue;
			}
			Hazel::Ref<Hazel::SubTexture2D> texture;
			if (s_TextureMap.find(titleType) != s_TextureMap.end()) {
				texture = s_TextureMap[titleType];
			}
			else {
				texture = m_TextureBush;
			}
			Hazel::Renderer2D::DrawQuad({x - m_MapWidth / 2.0f, m_MapHeight / 2.0f - y, 0.5f},{1.0f, 1.0f}, texture); // x - m_MapWidth / 2.0f,  y - m_MapHeight / 2.0f, 0.5f // 会导致y轴相反绘画
		}
	}

	//Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_TextureBush, 1.0f);
	//Hazel::Renderer2D::DrawQuad({ 1.0f, 0.0f, 0.9f }, { 1.0f, 1.0f }, m_TextureStair, 1.0f);
	//Hazel::Renderer2D::DrawQuad({ -1.0f, 0.0f, 0.9f }, { 1.0f, 2.0f }, m_TextureTree, 1.0f);
	Hazel::Renderer2D::EndScene();
#endif
}

void Sandbox2D::OnImGuiRender()
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
	// 事件
	m_CameraController.OnEvent(event);
}