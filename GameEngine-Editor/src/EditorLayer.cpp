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

		// ��Զ�����
		m_CameraController.SetZoomLevel(3.0f);

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		 
		// ����Ĭ�ϴ򿪳��� E:\AllWorkSpace1\GameEngineLightWeight\GameEngine-Editor\assets\scenes\PinkCube.scene
		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1) {
			auto sceneFilePath = commandLineArgs[1];
			SceneSerializer serializer(m_ActiveScene);
			serializer.DeSerialize(sceneFilePath);
		}

#if 0
		auto redsquare = m_ActiveScene->CreateEntity("Red Square Entity");
		redsquare.AddComponent<SpriteRendererComponent>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // ������ɫ
		redsquare.GetComponent<TransformComponent>().Translation = glm::vec3(2.0f, 0.0f, 0.0f);

		auto square = m_ActiveScene->CreateEntity("Green Square Entity");
		square.AddComponent<SpriteRendererComponent>(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		// ����ID
		m_SquareEntity = square;

		// ��ʼ�������ʵ��
		m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
		m_CameraEntity.AddComponent<CameraComponent>();
		m_CameraEntity.GetComponent<TransformComponent>().Translation = glm::vec3{ 0,0,10.0f };
		
		m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.primary = false; // �ڶ��������Ϊfalse

		class CameraController : public ScriptableEntity {
		public:
			virtual void OnCreate() override{
				//auto& tfc = GetComponent<TransformComponent>();
				//tfc.Translation.x = rand() % 10 - 5.0f;
			}
			virtual void OnDestroy() override {}
			virtual void OnUpdate(Timestep ts)override {
				// ��ȡ��ǰ����CameraController�ű���ʵ���TransformComponent���
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
		// ����resize����ÿһ֡���
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y)) {
			// ����֡��������С
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			// �������������ͶӰ
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			// �����༭�������߱ȣ����¼���ͶӰ����
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			// ���������ڵ��������߱ȣ����¼���ͶӰ����
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// ������۽�������wasd
		if (m_ViewportFocused) {
			m_CameraController.OnUpdate(ts);
		}
		// ����Ҫ���㣬ÿһ֡����Ҫˢ��
		m_EditorCamera.OnUpdate(ts);

		// ��Ⱦ��Ϣ��ʼ��
		Renderer2D::ResetStats();
		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			// ����Ⱦ�Ķ����ŵ�֡������
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();

			// ��-1���֡����ĵڶ�����ɫ������
			m_Framebuffer->ClearAttachment(1, -1);
		}
		{
			HZ_PROFILE_SCOPE("Renderer Draw");

			// Scene����
			//m_ActiveScene->OnUpdateRuntime(ts);
			m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

			// ����λ���ǣ���ǰλ�þ�����Ļ���Ͻ�(0,0)��λ��
			// 1.��ȡ��ǰ������������Ļ���Ͻ�(0,0)��λ��
			auto [mx, my] = ImGui::GetMousePos();
			// 2.������λ�ü�ȥviewport���ڵ����ϽǾ���λ��=��������viewport�������Ͻǵ�λ��
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;

			// 3.viewport���ڵ����½Ǿ���λ��-���Ͻǵľ���λ��=viewport���ڵ�λ��
			glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
			// ��תy,ʹ�����½ǿ�ʼ����(0,0)
			my = viewportSize.y - my;
			int mouseX = (int)mx;
			int mouseY = (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y) {
				//HZ_CORE_WARN("Mouse xy = {0} {1}", mouseX, mouseY);
				// 4.��ȡ֡����ڶ���������������
				int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
				HZ_CORE_WARN("Pixel data = {0}", pixelData);
				m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
			}

			// ���֡����
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
		// ����UI�����������С���
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWinSizeX; // �ָ�

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
		// ��Ⱦ
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

		// Imgui�����µ��Ӵ���
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
		ImGui::Begin("Viewport");
		// ������ô���ӣ�imgui�ṩ��api��ÿ֡��ȡ�ӿڵĴ�С��������������
		//auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		//auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		//auto viewportOffset = ImGui::GetWindowPos();
		//m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		//m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		// 1.�Ȼ�ȡViewport�ӿ����Ͻ���viewport�ӿڱ����������ƫ��λ�ã�0,24)- ������⣬��Ϊ������������ӿڵ����Ͻ�
		auto viewportOffset = ImGui::GetCursorPos();

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		//HZ_WARN("Focus:{0}, Hover:{1}", m_ViewportFocused, m_ViewportHovered);
		/*
			�޸�֮ǰ����˼�ǵ��������岢����ͣ������ϣ����ܽ����¼���������������ܽ����¼�
			bool canshu = !m_ViewportFocused || !m_ViewportHovered;
			m_ViewportFocused = true,  m_ViewportHovered = true; canshu = false, m_BlockEvents��false-> viewport��� �� �����¼�
			m_ViewportFocused = true,  m_ViewportHovered = false;canshu = true,  m_BlockEvents��true-> viewport��� �� �ܽ����¼�
			m_ViewportFocused = false, m_ViewportHovered = true; canshu = true,  m_BlockEvents��true-> viewport��� �� �ܽ����¼�
			m_ViewportFocused = false, m_ViewportHovered = false;canshu = true,  m_BlockEvents��true-> viewport��� �� �ܽ����¼�

			�޸�֮����˼�ǵ����û�е����岢��û����ͣ������ϣ��Ͳ������¼�������������ɽ����¼�
			bool canshu = !m_ViewportFocused && !m_ViewportHovered;
			m_ViewportFocused = true,  m_ViewportHovered = true; canshu = false, m_BlockEvents��false-> viewport��� �� �����¼�
			m_ViewportFocused = true,  m_ViewportHovered = false;canshu = false,  m_BlockEvents��true-> viewport��� �� �����¼�
			m_ViewportFocused = false, m_ViewportHovered = true; canshu = false,  m_BlockEvents��true-> viewport��� �� �����¼�
			m_ViewportFocused = false, m_ViewportHovered = false;canshu = true,  m_BlockEvents��true->  viewport��� �� �ܽ����¼�
		*/
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		// ��ȡ���Ӵ��ڵĴ�С
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		// imgui��Ⱦ֡�����еĶ�����
		// textureID�ǻ�����ID
		uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
		/*
			imgui��uvĬ�������½�Ϊ01�����½�Ϊ11�����Ͻ�Ϊ00�����Ͻ���10

			ImVec2(0, 1):�������Ͻǵ��uv�� 0 1
			ImVec2(1, 0):�������½ǵ��uv�� 1 0
			��Ϊ���ǻ��Ƶ�quad��uv�����½�Ϊ00�����½�10�����Ͻ�01�����Ͻ�11��
		*/
		ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));

		// 2.��ȡvieport�ӿڴ�С - �����������ĸ�
		auto windowSize = ImGui::GetWindowSize();
		// 3.��ȡ��ǰvieport�ӿڱ��������ϽǾ��뵱ǰ������Ļ���Ͻǣ�0,0����λ��
		ImVec2 minBound = ImGui::GetWindowPos();
		// 4.����viewport�ӿڵ����ϽǾ��뵱ǰ������Ļ���Ͻǣ�0,0����λ��
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;
		// 5. ����viewport�ӿڵ����½Ǿ��뵱ǰ������Ļ���Ͻǣ�0,0����λ��
		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y - viewportOffset.y };
		// 6. �������ϽǺ����½Ǿ���������Ļ���Ͻǵ�λ��
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

			// Camera - editor �༭ʱ�����������
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // ƽ�Ƶ�snap
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE) {// rotate�Ķ���
				snapValue = 45.0f;
			}
			float snapValues[3] = { snapValue, snapValue,snapValue };

			// �������˵�Ǵ�����Ӧ�������õ��滭������gizmos
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			// ���gizmos��ʹ�� ���� ˵���ƶ�
			if (ImGuizmo::IsUsing()) {
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				// ��������ת�����������ܻ������������
				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation; // ÿһ֡����û�����ƽǶȣ������ǹ̶���360������
				tc.Scale = scale;
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		// �¼�
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
				// ���浱ǰ����:Ҫ��һ����¼��ǰ������·����
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
				m_ViewportHovered ��Ϊ���ڱ���ӿڵ������رյ�ǰ��ʾ��gizmo
				��������&&�ǽ������
				1. �϶�gizmo�ƶ�һ��ʵ������һ��ʵ���ص�ʱͣ�£�����һ��ʵ���ڵ�ǰʵ�����棬�ٵ��gizmo���ƶ�ԭ��ʵ�壬��ô���ȡ��������һ��ʵ���ʵ��ID������һ��ʵ��ᱻѡ�У����л�gizmo��
				2. ��ʾ��gizmo������������leftalt�϶���ת���������gizmo����ʧ
			*/
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt)) {
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			}
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		// �����³��� ����δ���ɽ�� ��μ��س������Ὣ�³�����ʵ��͵�ǰ������ʵ��һ����ֵ�bug
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