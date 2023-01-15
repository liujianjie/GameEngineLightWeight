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
	Hazel::Ref<Hazel::Texture2D> m_SquareTexture;		// 棋盘纹理
	Hazel::Ref<Hazel::Framebuffer> m_Framebuffer;		// 帧缓冲


	glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };

	Hazel::Ref<Hazel::Texture2D> m_SpriteSheet;			// 纹理 sheet
	// 子纹理
	Hazel::Ref<Hazel::SubTexture2D> m_TextureStair, m_TextureTree, m_TextureBush;

	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;

	uint32_t m_MapWidth, m_MapHeight;
	std::unordered_map<char, Hazel::Ref<Hazel::SubTexture2D>> s_TextureMap;
};