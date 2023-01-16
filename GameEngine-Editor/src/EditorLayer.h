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
		Ref<Shader> m_FlatShader;			// shader¿‡ ÷∏’Î
		Ref<VertexArray> m_FlatVertexArray;
		Ref<Texture2D> m_SquareTexture;		// ∆Â≈ÃŒ∆¿Ì
		Ref<Framebuffer> m_Framebuffer;		// ÷°ª∫≥Â

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };


		glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		bool m_ViewportFocused = false, m_ViewportHovered = false;
	};
}
