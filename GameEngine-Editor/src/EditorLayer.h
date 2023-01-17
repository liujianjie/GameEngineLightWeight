#pragma once
#include "Hazel.h"
#include "ParticleSystem.h"

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
		OrthographicCameraController m_CameraController;
		Ref<Shader> m_FlatShader;			// shader类 指针
		Ref<VertexArray> m_FlatVertexArray;
		Ref<Texture2D> m_SquareTexture;		// 棋盘纹理
		Ref<Framebuffer> m_Framebuffer;		// 帧缓冲
		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;			// 实体，由scene创建的

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		bool m_ViewportFocused = false, m_ViewportHovered = false;
	};
}
