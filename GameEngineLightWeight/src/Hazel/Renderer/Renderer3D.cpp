#include "hzpch.h"
#include "Renderer3D.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "Hazel/Renderer/UniformBuffer.h"
#include "Texture.h"
#include "RenderCommand.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace Hazel {
	struct QuadVertex {
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		// Editor-only;
		int EntityID;
	};
	struct CircleVertex {
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;
		// Editor-only;
		int EntityID;
	};
	struct LineVertex {
		glm::vec3 Position;
		glm::vec4 Color;
		// Editor-only;
		int EntityID;
	};
	struct Renderer3DData {
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; // �����������

		// quad
		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Texture2D> WhiteTexture;

		// circle
		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		// Line
		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;

		// quad
		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		// circle
		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		// Line
		uint32_t LineVertexCount = 0;// ֻ��Ҫ�ṩ��������
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		float LineWidth = 2.0f;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;// 0 �Ÿ���ɫ����

		glm::vec4 QuadVertexPosition[4];

		Renderer3D::Statistics Stats;

		struct CameraData {
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		int DrawCubeVertexCount = 36;
		glm::vec3 Cubevertices[36] = {
			{-0.5f, -0.5f, -0.5f},
			{ 0.5f, -0.5f, -0.5f},
			{ 0.5f,  0.5f, -0.5f},
			{ 0.5f,  0.5f, -0.5f},
			{-0.5f,  0.5f, -0.5f},
			{-0.5f, -0.5f, -0.5f},

			{-0.5f, -0.5f,  0.5f},
			{ 0.5f, -0.5f,  0.5f},
			{ 0.5f,  0.5f,  0.5f},
			{ 0.5f,  0.5f,  0.5f},
			{-0.5f,  0.5f,  0.5f},
			{-0.5f, -0.5f,  0.5f},

			{-0.5f,  0.5f,  0.5f},
			{-0.5f,  0.5f, -0.5f},
			{-0.5f, -0.5f, -0.5f},
			{-0.5f, -0.5f, -0.5f},
			{-0.5f, -0.5f,  0.5f},
			{-0.5f,  0.5f,  0.5f},

			{ 0.5f,  0.5f,  0.5f},
			{ 0.5f,  0.5f, -0.5f},
			{ 0.5f, -0.5f, -0.5f},
			{ 0.5f, -0.5f, -0.5f},
			{ 0.5f, -0.5f,  0.5f},
			{ 0.5f,  0.5f,  0.5f},

			{-0.5f, -0.5f, -0.5f},
			{ 0.5f, -0.5f, -0.5f},
			{ 0.5f, -0.5f,  0.5f},
			{ 0.5f, -0.5f,  0.5f},
			{-0.5f, -0.5f,  0.5f},
			{-0.5f, -0.5f, -0.5f},

			{-0.5f,  0.5f, -0.5f},
			{ 0.5f,  0.5f, -0.5f},
			{ 0.5f,  0.5f,  0.5f},
			{ 0.5f,  0.5f,  0.5f},
			{-0.5f,  0.5f,  0.5f},
			{-0.5f,  0.5f, -0.5f}
		};
		glm::vec2 CubeTexPos[36] = {
			  {0.0f, 0.0f},
			  {1.0f, 0.0f},
			  {1.0f, 1.0f},
			  {1.0f, 1.0f},
			  {0.0f, 1.0f},
			  {0.0f, 0.0f},
						  
			  {0.0f, 0.0f},
			  {1.0f, 0.0f},
			  {1.0f, 1.0f},
			  {1.0f, 1.0f},
			  {0.0f, 1.0f},
			  {0.0f, 0.0f},
						  
			  {1.0f, 0.0f},
			  {1.0f, 1.0f},
			  {0.0f, 1.0f},
			  {0.0f, 1.0f},
			  {0.0f, 0.0f},
			  {1.0f, 0.0f},
						  
			  {1.0f, 0.0f},
			  {1.0f, 1.0f},
			  {0.0f, 1.0f},
			  {0.0f, 1.0f},
			  {0.0f, 0.0f},
			  {1.0f, 0.0f},
						  
			  {0.0f, 1.0f},
			  {1.0f, 1.0f},
			  {1.0f, 0.0f},
			  {1.0f, 0.0f},
			  {0.0f, 0.0f},
			  {0.0f, 1.0f},
						  
			  {0.0f, 1.0f},
			  {1.0f, 1.0f},
			  {1.0f, 0.0f},
			  {1.0f, 0.0f},
			  {0.0f, 0.0f},
			  {0.0f, 1.0f}
		};
	};
	static Renderer3DData s_Data;
	void Renderer3D::Init()
	{
		HZ_PROFILE_FUNCTION();

		// 0.��CPU���ٴ洢s_Data.MaxVertices����QuadVertex���ڴ�
		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
		// quad//////////////////////////////////////////////////////////
		// 1.������������
		s_Data.QuadVertexArray = VertexArray::Create();

		// 2.�������㻺����,����GPU����һ��s_Data.MaxVertices * sizeof(QuadVertex)��С���ڴ�
		// ��cpu��Ӧ����Ϊ�˴��䶥������
		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));

		// 2.1���ö��㻺��������
		s_Data.QuadVertexBuffer->SetLayout({
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
			{ShaderDataType::Int, "a_EntityID"}
			});

		// 1.1���ö�������ʹ�õĻ���������������������������ò���
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		// 3.��������
		//uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };
		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		// һ��quad��6��������012 230��456 674
		uint32_t offset = 0;
		//for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36) {
		//	quadIndices[i + 0] = offset + 0;
		//	quadIndices[i + 1] = offset + 1;
		//	quadIndices[i + 2] = offset + 2;

		//	quadIndices[i + 3] = offset + 2;
		//	quadIndices[i + 4] = offset + 3;
		//	quadIndices[i + 5] = offset + 0;

		//	offset += 4;
		//}
		for (uint32_t i = 0; i < s_Data.MaxIndices; i ++) {
			quadIndices[i] = i;

			offset++;
		}

		// 3.1��������������
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);

		// 1.2����������������������
		s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
		// cpu�ϴ���gpu���˿���ɾ��cpu���������ݿ���
		delete[] quadIndices;

		// circle//////////////////////////////////////////////////////////
		// 0.��CPU���ٴ洢s_Data.MaxVertices����QuadVertex���ڴ�
		s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];
		// 1.������������
		s_Data.CircleVertexArray = VertexArray::Create();

		// 2.�������㻺����,����GPU����һ��s_Data.MaxVertices * sizeof(QuadVertex)��С���ڴ�
		// ��cpu��Ӧ����Ϊ�˴��䶥������
		s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));

		// 2.1���ö��㻺��������
		s_Data.CircleVertexBuffer->SetLayout({
			{ShaderDataType::Float3, "a_WorldPosition"},
			{ShaderDataType::Float3, "a_LocalPosition"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float, "a_Thickness"},
			{ShaderDataType::Float, "a_Fade"},
			{ShaderDataType::Int, "a_EntityID"}
			});

		// 1.1���ö�������ʹ�õĻ���������������������������ò���
		s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);

		// 3.��������-��quadʹ�õ���ͬһ��������Ҫ���½�

		// 1.2����������������������
		s_Data.CircleVertexArray->SetIndexBuffer(quadIB); // ����д�����s_Data.QuadVertexArray����û�и�circle�Ķ���������������

		// Line//////////////////////////////////////////////////////////
		// 0.��CPU���ٴ洢s_Data.MaxVertices����QuadVertex���ڴ�
		s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];
		// 1.������������
		s_Data.LineVertexArray = VertexArray::Create();

		// 2.�������㻺����,����GPU����һ��s_Data.MaxVertices * sizeof(QuadVertex)��С���ڴ�
		// ��cpu��Ӧ����Ϊ�˴��䶥������
		s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));

		// 2.1���ö��㻺��������
		s_Data.LineVertexBuffer->SetLayout({
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Int, "a_EntityID"}
			});

		// 1.1���ö�������ʹ�õĻ���������������������������ò���
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);

		// 3.��������-Line����Ҫ����������

		// ����////////////////////////////////////////
		// ����һ����ɫTexture
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// 0�Ÿ���ɫ����
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++) {
			samplers[i] = i;
		}

		// shader ////////////////////////////////////////////////
		s_Data.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
		s_Data.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
		s_Data.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");

		// ��ʼ��///////////////////////////////////////////
		// ����quad�ĳ�ʼλ��
		
		//s_Data.QuadVertexPosition[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		//s_Data.QuadVertexPosition[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		//s_Data.QuadVertexPosition[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		//s_Data.QuadVertexPosition[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
		//for (int i = 0; i < 36; i++) {
		//	s_Data.QuadVertexPosition[i] = glm::vec4(vertices[i]);
		//}

		// ��0��λ�����������ͶӰ��ͼ����
		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
	}

	void Renderer3D::Shutdown()
	{
		HZ_PROFILE_FUNCTION();

		delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		HZ_PROFILE_FUNCTION();

		// ͶӰ����projection * ��ͼ����
		//glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		//s_Data.QuadShader->Bind();		// ��shader
		//s_Data.QuadShader->SetMat4("u_ViewProjection", viewProj);

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));
		StartBatch();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		HZ_PROFILE_FUNCTION();

		//glm::mat4 viewProj = camera.GetViewProjection();
		//s_Data.QuadShader->Bind();
		//s_Data.QuadShader->SetMat4("u_ViewProjection", viewProj);
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

		StartBatch();
	}

	void Renderer3D::BeginScene(const OrthographicCamera& camera)
	{
		HZ_PROFILE_FUNCTION();
		// �Ѿ�����vulkan��glsl����������Ǵ��
		//s_Data.QuadShader->Bind();		// ��shader
		//s_Data.QuadShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

		StartBatch();
	}

	void Renderer3D::EndScene()
	{
		HZ_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer3D::Flush()
	{
		if (s_Data.QuadIndexCount) {
			// ���㵱ǰ������Ҫ���ٸ���������
			uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;
			// ��ȡ����CPU�Ķ��������ϴ�OpenGL
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

			// ��Ӧi��texture�󶨵�i�������
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) {
				s_Data.TextureSlots[i]->Bind(i);
			}
			s_Data.QuadShader->Bind();
			// ���û滭����
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
			s_Data.Stats.DrawCalls++;
		}
		if (s_Data.CircleIndexCount) {
			// ���㵱ǰ������Ҫ���ٸ���������
			uint32_t dataSize = (uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase;
			// ��ȡ����CPU�Ķ��������ϴ�OpenGL
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);

			s_Data.CircleShader->Bind();
			// ���û滭����
			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
			s_Data.Stats.DrawCalls++;
		}
		if (s_Data.LineVertexCount) {
			// ���㵱ǰ������Ҫ���ٸ���������
			uint32_t dataSize = (uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase;
			// ��ȡ����CPU�Ķ��������ϴ�OpenGL
			s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);

			s_Data.LineShader->Bind();
			// �����ģ������������
			RenderCommand::SetLineWidth(s_Data.LineWidth);
			// ���û滭����
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
			s_Data.Stats.DrawCalls++;
		}
	}
	void Renderer3D::StartBatch()
	{
		// �൱�ڳ�ʼ����֡Ҫ���Ƶ������������ϴ��Ķ�������
		s_Data.QuadIndexCount = 0;
		// ָ�븳��
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.CircleIndexCount = 0;// �൱�ڳ�ʼ����֡Ҫ���Ƶ������������ϴ��Ķ�������
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;// ָ�븳��

		s_Data.LineVertexCount = 0;// �൱�ڳ�ʼ����֡Ҫ���Ƶ������������ϴ��Ķ�������
		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;// ָ�븳��

		// ������Ϣ����
		s_Data.TextureSlotIndex = 1;
	}
	void Renderer3D::NextBatch()
	{
		Flush();
		StartBatch();
	}
	void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}
	void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}
	void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		// ����transform
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}
	// ������
	void Renderer3D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, subtexture, tilingFactor, tintColor);
	}
	void Renderer3D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		const glm::vec2* textureCoords = subtexture->GetTexCoords();
		const Ref<Texture2D> texture = subtexture->GetTexture();

		if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
			NextBatch();
		}

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			// ��ǰ��������Ѿ��洢������ۣ���ֱ�Ӷ�ȡ
			if (*s_Data.TextureSlots[i].get() == *texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0.0f) {
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;// �ǵ�++
		}
		// ����transform
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		// quad�����½�Ϊ���
		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPosition[0];
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[0];
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPosition[1];
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[1];
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPosition[2];
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[2];
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPosition[3];
		s_Data.QuadVertexBufferPtr->Color = color;
		s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[3];
		s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
		s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		s_Data.QuadVertexBufferPtr++;

		s_Data.QuadIndexCount += 36;// ÿһ��quad��6������

		s_Data.Stats.QuadCount++;
	}
	///////////////////////////////////////////////////////////////////////
	// ���ķ���////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////
	void Renderer3D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		HZ_PROFILE_FUNCTION();
		if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
			NextBatch();
		}
		constexpr size_t quadVertexCount = 36;
		const float textureIndex = 0.0f; // ��ɫ����
		const float tilingFactor = 1.0f;
		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		// quad�����½�Ϊ���
		for (size_t i = 0; i < quadVertexCount; i++) {
			s_Data.QuadVertexBufferPtr->Position = transform * glm::vec4(s_Data.Cubevertices[i], 1.0f);
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = s_Data.CubeTexPos[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 36;// ÿһ��quad��6������

		s_Data.Stats.QuadCount++;
	}
	void Renderer3D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		HZ_PROFILE_FUNCTION();
		if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
			NextBatch();
		}
		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		constexpr size_t quadVertexCount =  36;
		float textureIndex = 0.0f;

		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			// ��ǰ��������Ѿ��洢������ۣ���ֱ�Ӷ�ȡ
			if (*s_Data.TextureSlots[i].get() == *texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}
		// ��������۾�Ҫ���õ��������
		if (textureIndex == 0.0f) {
			// ����Ѿ�������ǰ���������۾���Ⱦ��ǰ���Σ���ִ�п�����һ������
			if (s_Data.TextureSlotIndex >= Renderer3DData::MaxTextureSlots) {
				NextBatch();
			}
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;// �ǵ�++
		}
		for (uint32_t i = 0; i < quadVertexCount; i++) {
			// quad�����½�Ϊ���
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPosition[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;
			s_Data.QuadVertexBufferPtr++;
		}
		s_Data.QuadIndexCount += 36;// ÿһ��quad��6������

		s_Data.Stats.QuadCount++;
	}

	void Renderer3D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade, int entityID)
	{
		HZ_PROFILE_FUNCTION();
		// ����ע������Ϊ��circleһ�㲻�ᳬ������
		//if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
		//	NextBatch();
		//}
		constexpr size_t quadVertexCount =  36;
		// quad�����½�Ϊ���
		// ʹ�õ���quad������Ϣ
		for (size_t i = 0; i < quadVertexCount; i++) {
			s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPosition[i];
			s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPosition[i] * 2.0f; // 2 * 0.5 = 1
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = entityID;
			s_Data.CircleVertexBufferPtr++;
		}
		s_Data.CircleIndexCount += 36;// ÿһ��quad��6������

		s_Data.Stats.QuadCount++;
	}

	void Renderer3D::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID)
	{
		s_Data.LineVertexBufferPtr->Position = p0;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = p1;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;
	}

	void Renderer3D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
	{
		// position������λ��
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);// ���½�
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);// ���½�
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);// ���Ͻ�
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);// ���Ͻ�

		DrawLine(p0, p1, color, entityID);
		DrawLine(p1, p2, color, entityID);
		DrawLine(p2, p3, color, entityID);
		DrawLine(p3, p0, color, entityID);
	}

	void Renderer3D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
		{
			lineVertices[i] = transform * s_Data.QuadVertexPosition[i]; // quad�Ķ���λ��������rect�Ķ���λ��
		}
		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

	float Renderer3D::GetLineWidth()
	{
		return s_Data.LineWidth;
	}

	void Renderer3D::SetLineWidth(float width)
	{
		s_Data.LineWidth = width;
	}

	void Renderer3D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}
	void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		HZ_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount =  36;
		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
			NextBatch();
		}

		const float textureIndex = 0.0f; // ��ɫ����
		const float tilingFactor = 1.0f;

		// ����transform
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}
	void Renderer3D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}
	void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount =  36;
		constexpr glm::vec2 textureCoords[] = { {0.0f, 0.0f}, { 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
			NextBatch();
		}

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			// ��ǰ��������Ѿ��洢������ۣ���ֱ�Ӷ�ȡ
			if (*s_Data.TextureSlots[i].get() == *texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0.0f) {
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;// �ǵ�++
		}
		// ����transform
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}
	// ������
	void Renderer3D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, subtexture, tilingFactor, tintColor);
	}
	void Renderer3D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor)
	{
		HZ_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount =  36;
		const glm::vec2* textureCoords = subtexture->GetTexCoords();
		const Ref<Texture2D> texture = subtexture->GetTexture();

		if (s_Data.QuadIndexCount >= Renderer3DData::MaxIndices) {
			NextBatch();
		}

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			// ��ǰ��������Ѿ��洢������ۣ���ֱ�Ӷ�ȡ
			if (*s_Data.TextureSlots[i].get() == *texture.get()) {
				textureIndex = (float)i;
				break;
			}
		}
		if (textureIndex == 0.0f) {
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;// �ǵ�++
		}
		// ����transform
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), rotation, { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (uint32_t i = 0; i < quadVertexCount; i++) {
			// quad�����½�Ϊ���
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPosition[i];
			s_Data.QuadVertexBufferPtr->Color = tintColor;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 36;// ÿһ��quad��6������

		s_Data.Stats.QuadCount++;
	}
	void Renderer3D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if (src.Texture) {
			DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
		}
		else {
			DrawQuad(transform, src.Color, entityID);
		}
	}
	void Renderer3D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}
	Renderer3D::Statistics Renderer3D::GetStats()
	{
		return s_Data.Stats;
	}

}