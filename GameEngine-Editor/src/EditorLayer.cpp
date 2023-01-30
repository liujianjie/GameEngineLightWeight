#include "EditorLayer.h"
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Hazel/Core/KeyCodes.h"
#include <Hazel/Renderer/Renderer2D.h>
#include <chrono>
#include <string>
#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Utils/PlatformUtils.h"

#include "ImGuizmo.h"
#include "Hazel/Math/Math.h"

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
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		 
		// 设置默认打开场景 E:\AllWorkSpace1\GameEngineLightWeight\GameEngine-Editor\assets\scenes\PinkCube.scene
		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1) {
			auto sceneFilePath = commandLineArgs[1];
			SceneSerializer serializer(m_ActiveScene);
			serializer.DeSerialize(sceneFilePath);
		}

#if 0
		auto redsquare = m_ActiveScene->CreateEntity("Red Square Entity");
		redsquare.AddComponent<SpriteRendererComponent>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // 这是颜色
		redsquare.GetComponent<TransformComponent>().Translation = glm::vec3(2.0f, 0.0f, 0.0f);

		auto square = m_ActiveScene->CreateEntity("Green Square Entity");
		square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		// 保存ID
		m_SquareEntity = square;

		// 初始化摄像机实体
		m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
		m_CameraEntity.AddComponent<CameraComponent>();
		m_CameraEntity.GetComponent<TransformComponent>().Translation = glm::vec3{ 0,0,10.0f };
		
		m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.primary = false; // 第二个摄像机为false

		class CameraController : public ScriptableEntity {
		public:
			virtual void OnCreate() override{
				//auto& tfc = GetComponent<TransformComponent>();
				//tfc.Translation.x = rand() % 10 - 5.0f;
			}
			virtual void OnDestroy() override {}
			virtual void OnUpdate(Timestep ts)override {
				// 获取当前挂载CameraController脚本的实体的TransformComponent组件
				auto& tfc = GetComponent<TransformComponent>();
				float speed = 5.0f;
				if (Input::IsKeyPressed(KeyCode::A))
					tfc.Translation.x -= speed * ts;
				if (Input::IsKeyPressed(KeyCode::D))
					tfc.Translation.x += speed * ts;
				if (Input::IsKeyPressed(KeyCode::W))
					tfc.Translation.y += speed * ts;
				if (Input::IsKeyPressed(KeyCode::S))
					tfc.Translation.y -= speed * ts;
			}
		};
		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_SecondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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
		// 窗口resize，在每一帧检测
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y)) {
			// 调整帧缓冲区大小
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			// 调整正交摄像机投影
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			// 调整编辑摄像机宽高比，重新计算投影矩阵
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			// 调整场景内的摄像机宽高比，重新计算投影矩阵
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// 当焦点聚焦，才能wasd
		if (m_ViewportFocused) {
			m_CameraController.OnUpdate(ts);
		}
		// 不需要焦点，每一帧都需要刷新
		m_EditorCamera.OnUpdate(ts);

		// 渲染信息初始化
		Renderer2D::ResetStats();
		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			// 将渲染的东西放到帧缓冲中
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();

			// 用-1填充帧缓冲的第二个颜色缓冲区
			m_Framebuffer->ClearAttachment(1, -1);
		}
		{
			HZ_PROFILE_SCOPE("Renderer Draw");

			// Scene更新
			//m_ActiveScene->OnUpdateRuntime(ts);
			m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

			// 绝对位置是：当前位置距离屏幕左上角(0,0)的位置
			// 1.获取当前鼠标距离整个屏幕左上角(0,0)的位置
			auto [mx, my] = ImGui::GetMousePos();
			// 2.鼠标绝对位置减去viewport窗口的左上角绝对位置=鼠标相对于viewport窗口左上角的位置
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;

			// 3.viewport窗口的右下角绝对位置-左上角的绝对位置=viewport窗口的位置
			glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
			// 翻转y,使其左下角开始才是(0,0)
			my = viewportSize.y - my;
			int mouseX = (int)mx;
			int mouseY = (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y) {
				//HZ_CORE_WARN("Mouse xy = {0} {1}", mouseX, mouseY);
				// 4.读取帧缓冲第二个缓冲区的数据
				int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
				HZ_CORE_WARN("Pixel data = {0}", pixelData);
				m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
			}

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
		ImGuiStyle& style = ImGui::GetStyle();
		// 完善UI：设置面板最小宽度
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWinSizeX; // 恢复

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N")) {
					NewScene();
				}
				if (ImGui::MenuItem("Open...", "Ctrl+O")) {
					OpenScene();
				}
				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
					SaveSceneAs();
				}
				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		// 渲染
		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();

		ImGui::Begin("Stats");
		
		std::string name = "None";
		if (m_HoveredEntity) {
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		}
		ImGui::Text("Hovered Entity: %s", name.c_str());

		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::End();

		// Imgui创建新的子窗口
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
		ImGui::Begin("Viewport");
		// 不用这么复杂，imgui提供了api：每帧获取视口的大小，不包括标题栏
		//auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		//auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		//auto viewportOffset = ImGui::GetWindowPos();
		//m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		//m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		// 1.先获取Viewport视口左上角与viewport视口标题栏距离的偏移位置（0,24)- 必须放这，因为标题栏后就是视口的左上角
		auto viewportOffset = ImGui::GetCursorPos();

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		//HZ_WARN("Focus:{0}, Hover:{1}", m_ViewportFocused, m_ViewportHovered);
		/*
			修改之前：意思是当鼠标点击面板并且悬停在面板上，才能接受事件，其它情况均不能接收事件
			bool canshu = !m_ViewportFocused || !m_ViewportHovered;
			m_ViewportFocused = true,  m_ViewportHovered = true; canshu = false, m_BlockEvents：false-> viewport面板 能 接收事件
			m_ViewportFocused = true,  m_ViewportHovered = false;canshu = true,  m_BlockEvents：true-> viewport面板 不 能接收事件
			m_ViewportFocused = false, m_ViewportHovered = true; canshu = true,  m_BlockEvents：true-> viewport面板 不 能接收事件
			m_ViewportFocused = false, m_ViewportHovered = false;canshu = true,  m_BlockEvents：true-> viewport面板 不 能接收事件

			修改之后：意思是当鼠标没有点击面板并且没有悬停在面板上，就不接受事件，其它情况均可接收事件
			bool canshu = !m_ViewportFocused && !m_ViewportHovered;
			m_ViewportFocused = true,  m_ViewportHovered = true; canshu = false, m_BlockEvents：false-> viewport面板 能 接收事件
			m_ViewportFocused = true,  m_ViewportHovered = false;canshu = false,  m_BlockEvents：true-> viewport面板 能 接收事件
			m_ViewportFocused = false, m_ViewportHovered = true; canshu = false,  m_BlockEvents：true-> viewport面板 能 接收事件
			m_ViewportFocused = false, m_ViewportHovered = false;canshu = true,  m_BlockEvents：true->  viewport面板 不 能接收事件
		*/
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		// 获取到子窗口的大小
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		// imgui渲染帧缓冲中的东西。
		// textureID是缓冲区ID
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
		/*
			imgui的uv默认是左下角为01，右下角为11，左上角为00，右上角是10

			ImVec2(0, 1):设置左上角点的uv是 0 1
			ImVec2(1, 0):设置右下角点的uv是 1 0
			因为我们绘制的quad的uv是左下角为00，右下角10，左上角01，右上角11。
		*/
		ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

		// 2.获取vieport视口大小 - 包含标题栏的高
		auto windowSize = ImGui::GetWindowSize();
		// 3.获取当前vieport视口标题栏左上角距离当前整个屏幕左上角（0,0）的位置
		ImVec2 minBound = ImGui::GetWindowPos();
		// 4.计算viewport视口的左上角距离当前整个屏幕左上角（0,0）的位置
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;
		// 5. 计算viewport视口的右下角距离当前整个屏幕左上角（0,0）的位置
		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y - viewportOffset.y };
		// 6. 保存左上角和右下角距离整个屏幕左上角的位置
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		// ImGuizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1) {
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
			//ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Camera-runtime
			//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			//const auto& camera = cameraEntity.GetComponent<CameraComponent>().camera;
			//const glm::mat4& cameraProjection = camera.GetProjection();
			//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Camera - editor 编辑时的摄像机矩阵
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // 平移的snap
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) {// rotate的度数
				snapValue = 45.0f;
			}
			float snapValues[3] = { snapValue, snapValue,snapValue };

			// 这里可以说是传入相应参数，得到绘画出来的gizmos
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			// 如果gizmos被使用 或者 说被移动
			if (ImGuizmo::IsUsing()) {
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				// 用增量旋转，解决矩阵可能会造成万向锁。
				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation; // 每一帧增加没有限制角度，而不是固定在360度数。
				tc.Scale = scale;
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		// 事件
		m_CameraController.OnEvent(e);
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(HZ_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0) {
			return false;
		}
		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		switch (e.GetKeyCode()) {
			case Key::N: {
				if (control) {
					NewScene();
				}
				break;
			}
			case Key::O: {
				if (control) {
					OpenScene();
				}
				break;
			}
			case Key::S: {
				if (control && shift) {
					SaveSceneAs();
				}
				// 保存当前场景:要有一个记录当前场景的路径。
				//if (control) {

				//}
				break;
			}
			// Gizmos
			case Key::Q:
				m_GizmoType = -1;
				break;
			case Key::W:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Key::E:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			case Key::R:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
		}
		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft) {
			/*
				m_ViewportHovered 是为了在别的视口点击不会关闭当前显示的gizmo
				后面两个&&是解决下面
				1. 拖动gizmo移动一个实体与另一个实体重叠时停下，且另一个实体在当前实体上面，再点击gizmo想移动原先实体，那么会获取在上面另一个实体的实体ID，即另一个实体会被选中，会切换gizmo。
				2. 显示了gizmo，但是若按下leftalt拖动旋转摄像机，则gizmo会消失
			*/
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt)) {
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			}
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		// 创建新场景 ，这段代码可解决 多次加载场景，会将新场景的实体和当前场景的实体一起呈现的bug
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Game Scene (*.scene)\0*.scene\0");
		if (!filepath.empty()) {
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene); 

			SceneSerializer serializer(m_ActiveScene);
			serializer.DeSerialize(filepath);
		}
	}
	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Game Scene (*.scene)\0*.scene\0");
		if (!filepath.empty()) {
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
		}
	}
}