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
		// 渲染网格 flat
		float flatVertices[] = {
			-0.75f, -0.75f, 0.0f, 0.0f, 0.0f,
			0.75f, -0.75f, 0.0f,  1.0f,	0.0f,
			0.75f,  0.75f, 0.0f,	   1.0f,1.0f,
			-0.75f,  0.75f, 0.0f	, 0.0f, 1.0f
		};
		// 1.创建顶点数组
		s_Data->QuadVertexArray = (Hazel::VertexArray::Create());

		// 2.创建顶点缓冲区
		Hazel::Ref<Hazel::VertexBuffer> flatVB;
		flatVB.reset(Hazel::VertexBuffer::Create(flatVertices, sizeof(flatVertices)));

		// 2.1设置顶点缓冲区布局
		flatVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1顶点数组添加顶点缓冲区，并且在这个缓冲区中设置布局
		s_Data->QuadVertexArray->AddVertexBuffer(flatVB);

		// 3.索引缓冲
		uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> flatIB;
		flatIB.reset(Hazel::IndexBuffer::Create(flatIndices, sizeof(flatIndices) / sizeof(uint32_t)));

		// 1.2顶点数组设置索引缓冲区
		s_Data->QuadVertexArray->SetIndexBuffer(flatIB);

		s_Data->FlatColorShader = (Hazel::Shader::Create("assets/shaders/FlatColor.glsl"));

		// 纹理的shader
		s_Data->TextureShader = (Hazel::Shader::Create("assets/shaders/Texture.glsl"));
		/*
			把fragment的u_Texture要采样的纹理槽为0
			因为下面DrawQuad的代码，把m_SquareTexture->Bind,设置了m_SquareTexture的m_RenderID绑定在OpenGL的0槽上！
		*/
		s_Data->TextureShader->SetInt("u_Texture", 0);
	}

	void Hazel::Renderer2D::Shutdown()
	{
		delete s_Data; // 手动管理内存
	}

	void Hazel::Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		
		s_Data->FlatColorShader->Bind();		// 绑定shader
		// 上传矩阵数据给shader前，需要先绑定使用哪个shader！
		s_Data->FlatColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());


		s_Data->TextureShader->Bind();		// 绑定shader
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

		s_Data->FlatColorShader->Bind();		// 绑定shader
		s_Data->FlatColorShader->SetFloat4("u_Color", color);
		// 设置transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		s_Data->FlatColorShader->SetMat4("u_Transform", tranform);

		s_Data->QuadVertexArray->Bind();		// 绑定顶点数组
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture);
	}
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
	{
		// 绑定材质
		texture->Bind();

		// 设置transform
		glm::mat4 tranform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data->TextureShader->Bind();		// 绑定shader
		s_Data->TextureShader->SetMat4("u_Transform", tranform);

		s_Data->QuadVertexArray->Bind();		// 绑定顶点数组
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
}