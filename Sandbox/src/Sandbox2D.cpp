#include "Sandbox2D.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Hazel/Renderer/Renderer2D.h>

#include <chrono>
#include <string>
template<typename Fn>
class Timer {
public:
	Timer(const char* name, Fn&& func)
		:m_Name(name), m_Func(func), m_Stopped(false) 
	{
		m_StartTimepoint = std::chrono::high_resolution_clock::now();
	}
	~Timer() {
		if (!m_Stopped) {
			Stop();
		}
	}
	void Stop() {
		auto endTimepoint = std::chrono::high_resolution_clock::now();
		// microseconds 是微秒
		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
		
		m_Stopped = true;

		float duration = (end - start) * 0.001f;
		// 执行函数
		m_Func({ m_Name, duration });
	}
private:
	const char* m_Name;
	Fn m_Func;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
};
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](ProfileResult profileResult){ m_ProfileResults.push_back(profileResult);})

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
	HZ_PROFILE_FUNCTION();
	{
		HZ_PROFILE_SCOPE("m_CameraController OnUpdate");
		m_CameraController.OnUpdate(ts);
	}

	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::Clear();
	}
	{
		HZ_PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Hazel::Renderer2D::DrawQuad({-1.0f, 0.0f}, {0.8f,0.8f}, m_FlatColor);
		Hazel::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.8f }, {0.2f, 0.8f, 0.9f, 1.0f});
		Hazel::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_SquareTexture);
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
	// 事件
	m_CameraController.OnEvent(event);
}