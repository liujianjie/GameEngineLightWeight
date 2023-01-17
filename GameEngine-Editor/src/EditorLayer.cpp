#include "EditorLayer.h"
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Hazel/Renderer/Renderer2D.h>

#include <chrono>
#include <string>

namespace Hazel {

	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true)
	{
	}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		//Renderer2D::Init();
		m_SquareTexture = Texture2D::Create("assets/textures/Checkerboard.png");

		// 拉远摄像机
		m_CameraController.SetZoomLevel(3.0f);

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		auto square = m_ActiveScene->CreateEnitty("Square");
		square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		//m_ActiveScene->Reg().emplace<TransformComponent>(square); // 先要获取注册表才能添加组件
		//m_ActiveScene->Reg().emplace<SpriteRendererComponent>(square, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		// 保存ID
		m_SquareEntity = square;
	}
	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();

	}
	EditorLayer::~EditorLayer()
	{
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		HZ_PROFILE_FUNCTION();
		// 当焦点聚焦，才能wasd
		if (m_ViewportFocused) {
			m_CameraController.OnUpdate(ts);
		}
		// 渲染信息初始化
		Renderer2D::ResetStats();
		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			// 将渲染的东西放到帧缓冲中
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();
		}
		{
			HZ_PROFILE_SCOPE("Renderer Draw");
			//static float rotation = 0.0f;
			//rotation += ts * 50.0f;

			Renderer2D::BeginScene(m_CameraController.GetCamera());
			// Scene更新
			m_ActiveScene->OnUpdate(ts);
			//Renderer2D::DrawrRotatedQuad({ 1.0f, 0.5f }, { 0.8f, 0.8f }, glm::radians(30.0), m_FlatColor);
			//Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, m_FlatColor);
			//Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.8f }, { 0.2f, 0.8f, 0.9f, 1.0f });
			//Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 20.0f, 20.0f }, m_SquareTexture, 10.0f);
			//Renderer2D::DrawRotatedQuad({ -0.5f, -1.5f, 0.0f }, { 1.0f, 1.0f }, glm::radians(rotation), m_SquareTexture, 20.0f);
			//Renderer2D::EndScene();

			//// 开启新的绘制，会重置绘制内存
			//Renderer2D::BeginScene(m_CameraController.GetCamera());
			//for (float y = -5.0f; y < 5.0f; y += 0.5f) {
			//	for (float x = -5.0f; x < 5.0f; x += 0.5f)
			//	{
			//		glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f , (y + 5.0f) / 10.0f , 0.7f };
			//		Renderer2D::DrawQuad({ x, y }, { 0.45f, 0.45f }, color);
			//	}
			//}
			Renderer2D::EndScene();
			// 解绑帧缓冲
			m_Framebuffer->Unbind();
		}
	}
	void EditorLayer::OnImGuiRender()
	{
		HZ_PROFILE_FUNCTION();
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::Begin("Settings");
		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		if (m_SquareEntity) {
			ImGui::Separator();
			ImGui::Text("%s", m_SquareEntity.GetComponent<TagComponent>().Tag);
			auto& squareColor = m_SquareEntity.GetComponent<SpriteRendererComponent>().Color;
			//auto& squareColor = m_ActiveScene->Reg().get<SpriteRendererComponent>(m_SquareEntity).Color;

			ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
			ImGui::Separator();
		}
		ImGui::End();

		// Imgui创建新的子窗口
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		//HZ_WARN("Focus:{0}, Hover:{1}", m_ViewportFocused, m_ViewportHovered);
		/*
			bool canshu = !m_ViewportFocused || !m_ViewportHovered;
			m_ViewportFocused = true,  m_ViewportHovered = true; canshu = false, m_BlockEvents：false-> viewport面板 能 接收滚轮事件
			m_ViewportFocused = false, m_ViewportHovered = true;canshu = true,   m_BlockEvents：true-> viewport面板 不 能接收滚轮事件
			m_ViewportFocused = true,  m_ViewportHovered = true; canshu = true,  m_BlockEvents：true-> viewport面板 不 能接收滚轮事件
		*/
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

		// 获取到子窗口的大小
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize) &&
			viewportPanelSize.x > 0 && viewportPanelSize.y > 0) { // 改变了窗口大小
			// 调整帧缓冲区大小
			m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);

			m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			// 调整摄像机投影
			m_CameraController.OnResize(viewportPanelSize.x, viewportPanelSize.y);
		}

		// imgui渲染帧缓冲中的东西
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		/*
			imgui的uv默认是左下角为01，右下角为11，左上角为00，右上角是10

			ImVec2(0, 1):设置左上角点的uv是 0 1
			ImVec2(1, 0):设置右下角点的uv是 1 0
			因为我们绘制的quad的uv是左下角为00，右下角10，左上角01，右上角11。
		*/
		ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		// 事件
		m_CameraController.OnEvent(event);
	}

}