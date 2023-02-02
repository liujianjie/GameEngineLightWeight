#pragma once
#include "Hazel.h"
#include "ParticleSystem.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Hazel/Renderer/EditorCamera.h"

namespace Hazel {
	class EditorLayer :public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer();
		virtual void OnAttach() override;
		virtual void OnDetach()override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& event) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
		void SaveCurScene();// 保存当前场景

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();
		// 复制实体
		void OnDuplicateEntity();

		// UI Panels
		void UI_Toolbar();
	private:
		OrthographicCameraController m_CameraController;
		Ref<Shader> m_FlatShader;			// shader类 指针
		Ref<VertexArray> m_FlatVertexArray;
		Ref<Texture2D> m_SquareTexture;		// 棋盘纹理
		Ref<Framebuffer> m_Framebuffer;		// 帧缓冲
		// 当前活动场景
		Ref<Scene> m_ActiveScene;
		// 当前编辑场景
		Ref<Scene> m_EditorScene;
		Entity m_SquareEntity;			// 实体，由scene创建的
		Entity m_CameraEntity;			// 摄像机实体
		Entity m_SecondCamera;			// 摄像机实体
		
		Entity m_HoveredEntity;			// 鼠标当前位置的实体

		bool m_PrimaryCamera = true;		

		EditorCamera m_EditorCamera;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };


		bool m_ViewportFocused = false, m_ViewportHovered = false;

		// 
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		// imguizmo绘画的类型
		int m_GizmoType = 0;

		// 为了获取鼠标位置-得到窗口的边界
		glm::vec2 m_ViewportBounds[2]; // [0]是左上角的坐标，[1]是右下角的坐标

		glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		// 播放暂停按钮
		enum class SceneState {
			Edit = 0, Play = 1
		};
		SceneState m_SceneState = SceneState::Edit;
		Ref<Texture2D> m_IconPlay, m_IconStop;
	};
}
