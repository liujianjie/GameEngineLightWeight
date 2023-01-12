#include "hzpch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RenderCommand.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Hazel {
	struct QuadVertex {
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
	};
	struct Renderer2DData{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
	};
	static Renderer2DData s_Data;
	void Hazel::Renderer2D::Init()
	{
		HZ_PROFILE_FUNCTION();

		// 0.��CPU���ٴ洢s_Data.MaxVertices����QuadVertex���ڴ�
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		// 1.������������
		s_Data.QuadVertexArray = VertexArray::Create();

		// 2.�������㻺����,����GPU����һ��s_Data.MaxVertices * sizeof(QuadVertex)��С���ڴ�
		// ��cpu��Ӧ����Ϊ�˴��䶥������
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));

		// 2.1���ö��㻺��������
		s_Data.QuadVertexBuffer->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float4, "a_Color"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1����������Ӷ��㻺��������������������������ò���
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		// 3.��������
		//uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };
		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
		
		// һ��quad��6��������012 230��456 674
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;
										
			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> flatIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);

		// 1.2����������������������
		s_Data.QuadVertexArray->SetIndexBuffer(flatIB);
		// cpu�ϴ���gpu���˿���ɾ��cpu���������ݿ���
		delete[] quadIndices;

		//s_Data.FlatColorShader = (Hazel::Shader::Create("assets/shaders/FlatColor.glsl"));

		// �����shader
		s_Data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		/*
			��fragment��u_TextureҪ�����������Ϊ0
			��Ϊ����DrawQuad�Ĵ��룬��m_SquareTexture->Bind,������m_SquareTexture��m_RenderID����OpenGL��0���ϣ�
		*/
		s_Data.TextureShader->SetInt("u_Texture", 0);

		// ����һ����ɫTexture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
	}

	void Hazel::Renderer2D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

	}

	void Hazel::Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		s_Data.TextureShader->Bind();		// ��shader
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		// �൱�ڳ�ʼ����֡Ҫ���Ƶ������������ϴ��Ķ�������
		s_Data.QuadIndexCount = 0;
		// ָ�븳��
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
	}

	void Hazel::Renderer2D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		// ���㵱ǰ������Ҫ���ٸ���������
		uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
		// ��ȡ����CPU�Ķ��������ϴ�OpenGL
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
	}

	void Hazel::Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Hazel::Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		// quad�����½�Ϊ���
		s_Data.QuadVertexBufferPtr->Position = position;
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = { position.x + size.x, position.y + size.y, 0.0f };
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = { position.x, position.y +size.y , 0.0f };
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 6;
		//s_Data.TextureShader->SetFloat4("u_Color", color);
		//s_Data.TextureShader->SetFloat("u_TilingFactor", 1.0f);
		// �󶨲���
		//s_Data.WhiteTexture->Bind();

		// ����transform
		/*glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.TextureShader->SetMat4("u_Transform", tranform);

		s_Data.QuadVertexArray->Bind();		// �󶨶�������
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
	}
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		s_Data.TextureShader->SetFloat4("u_Color", tintColor);
		s_Data.TextureShader->SetFloat("u_TilingFactor", tilingFactor);
		// �󶨲���
		texture->Bind();

		// ����transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.TextureShader->SetMat4("u_Transform", tranform);

		s_Data.QuadVertexArray->Bind();		// �󶨶�������
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
	}
	void Renderer2D::DrawrRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawrRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
	}
	void Renderer2D::DrawrRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		s_Data.TextureShader->SetFloat4("u_Color", color);
		s_Data.TextureShader->SetFloat("u_TilingFactor", 1.0f);

		// �󶨲���
		s_Data.WhiteTexture->Bind();

		// ����transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.TextureShader->SetMat4("u_Transform", tranform);

		s_Data.QuadVertexArray->Bind();		// �󶨶�������
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
	}
	void Renderer2D::DrawrRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawrRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}
	void Renderer2D::DrawrRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		s_Data.TextureShader->SetFloat4("u_Color", tintColor);
		s_Data.TextureShader->SetFloat("u_TilingFactor", tilingFactor);
		// �󶨲���
		texture->Bind();

		// ����transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.TextureShader->SetMat4("u_Transform", tranform);

		s_Data.QuadVertexArray->Bind();		// �󶨶�������
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);
	}
}