#pragma once
#include "Hazel.h"
#include "ParticleSystem.h"
#include "Panels/SceneHierarchyPanel.h"

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
		void NewScene();
		void OpenScene();
		void SaveSceneAs();
	private:
		OrthographicCameraController m_CameraController;
		Ref<Shader> m_FlatShader;			// shader�� ָ��
		Ref<VertexArray> m_FlatVertexArray;
		Ref<Texture2D> m_SquareTexture;		// ��������
		Ref<Framebuffer> m_Framebuffer;		// ֡����
		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;			// ʵ�壬��scene������
		Entity m_CameraEntity;			// �����ʵ��
		Entity m_SecondCamera;			// �����ʵ��

		bool m_PrimaryCamera = true;		

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		// 
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};
}
