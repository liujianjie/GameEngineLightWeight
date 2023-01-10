#pragma once
#include "Hazel.h"

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
	Hazel::Ref<Hazel::Shader> m_FlatShader;			// shader�� ָ��
	Hazel::Ref<Hazel::VertexArray> m_FlatVertexArray;

	glm::vec4 m_FlatColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};