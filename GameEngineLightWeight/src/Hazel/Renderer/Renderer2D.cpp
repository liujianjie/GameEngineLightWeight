#include "hzpch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {
	static struct Renderer2DStorage{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;
		Ref<Shader> TextureShader;
	};
	static Renderer2DStorage* s_Data;
	void Hazel::Renderer2D::Init()
	{
		s_Data = new Renderer2DStorage();
		// ��Ⱦ���� flat
		float flatVertices[] = {
			-0.75f, -0.75f, 0.0f, 0.0f, 0.0f,
			0.75f, -0.75f, 0.0f,  1.0f,	0.0f,
			0.75f,  0.75f, 0.0f,	   1.0f,1.0f,
			-0.75f,  0.75f, 0.0f	, 0.0f, 1.0f
		};
		// 1.������������
		s_Data->QuadVertexArray = (Hazel::VertexArray::Create());

		// 2.�������㻺����
		Hazel::Ref<Hazel::VertexBuffer> flatVB;
		flatVB.reset(Hazel::VertexBuffer::Create(flatVertices, sizeof(flatVertices)));

		// 2.1���ö��㻺��������
		flatVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1����������Ӷ��㻺��������������������������ò���
		s_Data->QuadVertexArray->AddVertexBuffer(flatVB);

		// 3.��������
		uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> flatIB;
		flatIB.reset(Hazel::IndexBuffer::Create(flatIndices, sizeof(flatIndices) / sizeof(uint32_t)));

		// 1.2����������������������
		s_Data->QuadVertexArray->SetIndexBuffer(flatIB);

		s_Data->FlatColorShader = (Hazel::Shader::Create("assets/shaders/FlatColor.glsl"));

		// �����shader
		s_Data->TextureShader = (Hazel::Shader::Create("assets/shaders/Texture.glsl"));
		/*
			��fragment��u_TextureҪ�����������Ϊ0
			��Ϊ����DrawQuad�Ĵ��룬��m_SquareTexture->Bind,������m_SquareTexture��m_RenderID����OpenGL��0���ϣ�
		*/
		s_Data->TextureShader->SetInt("u_Texture", 0);
	}

	void Hazel::Renderer2D::Shutdown()
	{
		delete s_Data; // �ֶ������ڴ�
	}

	void Hazel::Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		
		s_Data->FlatColorShader->Bind();		// ��shader
		// �ϴ��������ݸ�shaderǰ����Ҫ�Ȱ�ʹ���ĸ�shader��
		s_Data->FlatColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());


		s_Data->TextureShader->Bind();		// ��shader
		s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Hazel::Renderer2D::EndScene()
	{
	}

	void Hazel::Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Hazel::Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{

		s_Data->FlatColorShader->Bind();		// ��shader
		s_Data->FlatColorShader->SetFloat4("u_Color", color);
		// ����transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->FlatColorShader->SetMat4("u_Transform", tranform);

		s_Data->QuadVertexArray->Bind();		// �󶨶�������
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture);
	}
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		// �󶨲���
		texture->Bind();

		// ����transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data->TextureShader->Bind();		// ��shader
		s_Data->TextureShader->SetMat4("u_Transform", tranform);

		s_Data->QuadVertexArray->Bind();		// �󶨶�������
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
}