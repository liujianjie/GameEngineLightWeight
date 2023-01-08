#include "hzpch.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Hazel {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::Init()
	{
		RenderCommand::Init();
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Renderer::EndScene()
	{
	}
	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, glm::mat4 transform)
	{
		vertexArray->Bind(); // �󶨶�������

		shader->Bind();		// ��shader
		// �ϴ��������ݵ�glsl
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		RenderCommand::DrawIndexed(vertexArray);
	}
}