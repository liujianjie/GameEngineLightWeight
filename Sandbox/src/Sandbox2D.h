#pragma once
#include "Hazel.h"
#include "ParticleSystem.h"

class Sandbox2D :public Hazel::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D();
	virtual void OnAttach() override;
	virtual void OnDetach()override;

	virtual void OnUpdate(Hazel::Timestep ts) override;
	virtual void OnImgGuiRender() override;
	virtual void OnEvent(Hazel::Event& event) override;
private:
	Hazel::OrthographicCameraController m_CameraController;
	Hazel::Ref<Hazel::Shader> m_FlatShader;			// shader类 指针
	Hazel::Ref<Hazel::VertexArray> m_FlatVertexArray;
	Hazel::Ref<Hazel::Texture2D> m_SquareTexture;		// 纹理
	Hazel::Ref<Hazel::Texture2D> m_TextureAltas;		// 纹理

	glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;
};