#include "hzpch.h"
#include "Renderer.h"


namespace Hazel {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}
	void Renderer::EndScene()
	{
	}
	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, glm::mat4 transform)
	{
		vertexArray->Bind(); // �󶨶�������

		shader->Bind();		// ��shader
		// �ϴ��������ݵ�glsl
		shader->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);

		shader->UploadUniformMat4("u_Transform", transform);

		RenderCommand::DrawIndexed(vertexArray);
	}
}