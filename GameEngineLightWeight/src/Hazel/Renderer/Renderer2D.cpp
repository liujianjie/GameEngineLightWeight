#include "hzpch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include <Platform/OpenGL/OpenGLShader.h>

namespace Hazel {
	static struct Renderer2DStorage{
		Ref<VertexArray> QuadVertexArray;
		Ref<Shader> FlatColorShader;
	};
	static Renderer2DStorage* s_Data;
	void Hazel::Renderer2D::Init()
	{
		s_Data = new Renderer2DStorage();
		// 渲染网格 flat
		float flatVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			0.75f, -0.75f, 0.0f,
			0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		// 1.创建顶点数组
		s_Data->QuadVertexArray = (Hazel::VertexArray::Create());

		// 2.创建顶点缓冲区
		Hazel::Ref<Hazel::VertexBuffer> flatVB;
		flatVB.reset(Hazel::VertexBuffer::Create(flatVertices, sizeof(flatVertices)));

		// 2.1设置顶点缓冲区布局
		flatVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"}
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
	}

	void Hazel::Renderer2D::Shutdown()
	{
		delete s_Data; // 手动管理内存
	}

	void Hazel::Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		// 上传矩阵数据到glsl
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_Transform", glm::mat4(1.0f));
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
		s_Data->QuadVertexArray->Bind();		// 绑定顶点数组
		s_Data->FlatColorShader->Bind();		// 绑定shader
		std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformFloat4("u_Color", color);
		RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
	}
}