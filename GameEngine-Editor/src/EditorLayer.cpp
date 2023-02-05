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

	extern const std::filesystem::path g_AssetPath;

	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true)
	{
	}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		//Renderer2D::Init();
		m_SquareTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_IconSimulate = Texture2D::Create("Resources/Icons/SimulateButton.png");
		m_IconStop = Texture2D::Create("Resources/Icons/StopButton.png");

		// ��Զ�����
		m_CameraController.SetZoomLevel(3.0f);

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		 
		// ����Ĭ�ϴ򿪳��� E:\AllWorkSpace1\GameEngineLightWeight\GameEngine-Editor\assets\scenes\PinkCube.scene
		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1) {
			auto sceneFilePath = commandLineArgs[1];
			SceneSerializer serializer(m_EditorScene);
			serializer.DeSerialize(sceneFilePath);
			m_ActiveScene = m_EditorScene;
		}
		// ��ǰ�������ǻ����
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

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

		// ��Ⱦ��Ϣ��ʼ��
		Renderer2D::ResetStats();
		{
			HZ_PROFILE_SCOPE("Renderer Prep");
			// ����Ⱦ�Ķ����ŵ�֡������
			m_Framebuffer->Bind();
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			//RenderCommand::SetClearColor({ 1.0f, 1.0f, 1.0f, 1 });
			RenderCommand::Clear();

			// ��-1���֡����ĵڶ�����ɫ������
			m_Framebuffer->ClearAttachment(1, -1);
		}
		{
			HZ_PROFILE_SCOPE("Renderer Draw");

			// Scene����
			switch (m_SceneState) {
				case SceneState::Edit: {
					// ������۽�������wasd
					if (m_ViewportFocused) {
						m_CameraController.OnUpdate(ts);
					}
					// ����Ҫ���㣬ÿһ֡����Ҫˢ��
					m_EditorCamera.OnUpdate(ts);
					m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
					break;
				}
				case SceneState::Simulate: {
					// �����Ҫ����
					m_EditorCamera.OnUpdate(ts);
					m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
					break;
				}
				case SceneState::Play: {
					m_ActiveScene->OnUpdateRuntime(ts);
					break;
				}
			}

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
			// Debug�Ļ滭��
			OnOverlayRender();

			// ���֡����
			m_Framebuffer->Unbind();
		}
	}
	void EditorLayer::OnOverlayRender()
	{	// ������ͬ�����
		if (m_SceneState == SceneState::Play) {// Playģʽ�����������
			Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
			if (!camera) {
				return;	// �Ҳ������˳�
			}
			Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().camera, camera.GetComponent<TransformComponent>().GetTransform());
		}
		else {
			Renderer2D::BeginScene(m_EditorCamera);
		}
		if (m_ShowPhysicsColliders) {
			// ��Χ��������Ӧ������,��Χ�е�transform����������ƽ�ơ���ת�����š�
			// Box Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view) {
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);
					// 0.001fZ��ƫ����
					glm::vec3 translation = tc.Translation + glm::vec3(bc2d.Offset, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f); // ע��bc2d.Size�����2��������Сһ��

					// Cherno�Ĵ��� ��BUG
					//glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
					//	* glm::rotate(glm::mat4(1.0f), tc.Rotation, glm::vec3(0.0f, 0.0f, 1.0f))// Χ��z��ת�ĽǶ�
					//	* glm::scale(glm::mat4(1.0f), scale);

					// Ӧ�ĳ�
					// ��һ��rotation���㷽ʽ����bug����ת�෴�������
					//glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), tc.Rotation.x, { 1,0,0 })
					//    * glm::rotate(glm::mat4(1.0f), tc.Rotation.y, { 0, 1, 0 })
					//    * glm::rotate(glm::mat4(1.0f), tc.Rotation.z, { 0, 0, 1 });
					// �ڶ���rotation���㷽ʽ ����Ԫ����þ���
					glm::mat4 rotation = glm::toMat4(glm::quat(tc.Rotation));

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* rotation
						* glm::scale(glm::mat4(1.0f), scale);

					Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));// ��ɫ�İ�Χ��
				}
			}
			// Circle Colliders
			{
				auto view = m_ActiveScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view) {
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);
					// 0.001fZ��ƫ����
					glm::vec3 translation = tc.Translation + glm::vec3(cc2d.Offset, 0.001f);
					glm::vec3 scale = tc.Scale * glm::vec3(cc2d.Radius * 2);// ע��cc2d.Radius�����2��������Сһ��

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation)
						* glm::scale(glm::mat4(1.0f), scale);

					Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.01f);// ��ɫ�İ�Χ��, �������������Ƴ���Բ��
				}
			}
		}
		Renderer2D::EndScene();
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
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					SaveCurScene();
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

		// ���ô���
		ImGui::Begin("Settings");
		ImGui::Checkbox("Show physics colliders", &m_ShowPhysicsColliders);
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
		// �����ڴ��ӿ��ϷŹ�����ֵ��On target candidates���Ϸ�Ŀ��
		if (ImGui::BeginDragDropTarget()) {
			// ��Ϊ�������ݿ���Ϊ�գ���Ҫif�ж�
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(std::filesystem::path(g_AssetPath) / path);
			}
			ImGui::EndDragDropTarget();
		}

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

		UI_Toolbar();

		ImGui::End();
	}

	void EditorLayer::UI_Toolbar() {
		// padding
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		// ��ťͼƬ͸������
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		// ��ť���hover��click�в�ͬЧ��
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		
		// ûɶ�ã�m_ActiveScene����true��ʵ�ֲ��˿ճ�������Play��Simulation
		bool toolbarEnabled = (bool)m_ActiveScene;
		ImVec4 tintColor = ImVec4(1, 1, 1, 1);
		if (!toolbarEnabled)
			tintColor.w = 0.5;

		// ��ť��Ӧ���ڴ�С���Ŵ�ʱӦʹ�����Բ�ֵ��֤����ôģ��
		float size = ImGui::GetWindowHeight() - 4.0f;
		{
			Ref<Texture2D> icon = (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) ? m_IconPlay : m_IconStop;
			// ��ťӦ�����м�
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));// ���ð�ť��xλ��
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor)
				&& toolbarEnabled) { // ͼ����Ի�����������Ĵ��벻��ִ�С���5��������paddingΪ0������û����
			//if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size))) {
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Simulate) {
					OnScenePlay();
				}
				else if (m_SceneState == SceneState::Play) {
					OnSceneStop();
				}
			}
		}
		ImGui::SameLine();
		{
			Ref<Texture2D> icon = ( m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play)? m_IconSimulate : m_IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) 
					&& toolbarEnabled) { // ͼ����Ի�����������Ĵ��벻��ִ��
				if (m_SceneState == SceneState::Edit || m_SceneState == SceneState::Play) {
					OnSceneSimulate();
				}
				else if (m_SceneState == SceneState::Simulate) {
					OnSceneStop();
				}
			}
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
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
			// Scene ����
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
				if (control) {
					SaveCurScene();
				}
				break;
			}
			// ����ʵ��
			case Key::D:
				if (control) {
					OnDuplicateEntity();
				}
				break;
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
			OpenScene(filepath);
		}
	}
	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		// ���е�ʱ����³�������ֹͣ��ǰ����
		if (m_SceneState != SceneState::Edit) {
			OnSceneStop();
		}
		// �ж��Ƿ�Ϊscene�ļ�
		if (path.extension().string() != ".scene") {
			HZ_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}
		// �����ǰ�г������ȱ���
		if (!m_ActiveScene->GetCurFilePath().empty()) {
			SerializeScene(m_ActiveScene, m_ActiveScene->GetCurFilePath());
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (serializer.DeSerialize(path.string())) {
			m_EditorScene = newScene;		
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			// ��ǰ������Ǳ༭����
			m_ActiveScene = m_EditorScene;
			// ��ǰ�������Ǳ༭����----------------------------
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		}
	}
	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Game Scene (*.scene)\0*.scene\0");
		if (!filepath.empty()) {
			if (filepath.find(".scene") == std::string::npos) {
				filepath.append(".scene");
			}
			SerializeScene(m_ActiveScene, filepath);
		}
	}
	void EditorLayer::SaveCurScene()
	{
		if (!m_ActiveScene->GetCurFilePath().empty()) {
			SerializeScene(m_ActiveScene, m_ActiveScene->GetCurFilePath());
		}
		else {
			SaveSceneAs();
		}
	}
	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}
	void EditorLayer::OnScenePlay()
	{
		if (m_SceneState == SceneState::Simulate) {
			OnSceneStop();// ֹͣ����,����
		}
		m_SceneState = SceneState::Play;

		// �����³����������
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart(); // �������³����Ϳ�ʼ����
		// ��ǰ���������³���----------------------------
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnSceneSimulate()
	{
		if (m_SceneState == SceneState::Play) {
			OnSceneStop(); // ֹͣ����
		}
		m_SceneState = SceneState::Simulate;

		// �����³����������
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart(); // �������³����Ϳ�ʼ����
		// ��ǰ���������³���----------------------------
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnSceneStop()
	{
		if (m_SceneState == SceneState::Play) {
			m_ActiveScene->OnRuntimeStop();// ֹͣ����,����
		}
		else if (m_SceneState == SceneState::Simulate) {
			m_ActiveScene->OnSimulationStop();// ֹͣ����,����
		}
		m_SceneState = SceneState::Edit;

		// ��ǰ������Ǳ༭����
		m_ActiveScene = m_EditorScene;
		// ��ǰ�������Ǳ༭����----------------------------
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnDuplicateEntity()
	{
		// �༭����ʱ ���Ը���ʵ��
		if (m_SceneState != SceneState::Edit) {
			return;
		}
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity) {
			m_EditorScene->DuplicateEntity(selectedEntity);
		}
	}
}