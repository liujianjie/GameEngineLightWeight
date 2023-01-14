#include <Hazel.h>
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Core/EntryPoint.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sandbox2D.h"

class ExampleLayer :public Hazel::Layer {
public:
	// , m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	ExampleLayer() : Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		// ��Ⱦ���� flat
		float flatVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		// 1.������������
		//m_FlatVertexArray.reset(Hazel::VertexArray::Create());
		m_FlatVertexArray = Hazel::VertexArray::Create();
		// 2.�������㻺����
		Hazel::Ref<Hazel::VertexBuffer> flatVB;
		flatVB = (Hazel::VertexBuffer::Create(flatVertices, sizeof(flatVertices)));

		// 2.1���ö��㻺��������
		flatVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"}
			});

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_FlatVertexArray->AddVertexBuffer(flatVB);

		// 3.��������
		uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };

		::Hazel::Ref<Hazel::IndexBuffer> flatIB;
		flatIB = (Hazel::IndexBuffer::Create(flatIndices, sizeof(flatIndices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_FlatVertexArray->SetIndexBuffer(flatIB);

		// 4.��ɫ��
		std::string flatShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main(){
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}			
		)";
		std::string flatShaderfragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			
			uniform vec3 u_Color;
				
			void main(){
				color = vec4(u_Color, 1.0f);	
			}			
		)";
		//m_FlatShader.reset(Hazel::Shader::Create(flatShaderVertexSrc, flatShaderfragmentSrc));
		m_FlatShader = Hazel::Shader::Create("flatPosColorShader", flatShaderVertexSrc, flatShaderfragmentSrc);


		// ������Ⱦ�����ε�������ɫ���������������ǻ������½�Ϊ������ģ�����Ϊ0������Ϊ1������(-0.5f, 0.5f)������������(0.0f, 1.0f)
		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		// 1.������������
		//m_SquareVertexArray.reset(Hazel::VertexArray::Create());
		m_SquareVertexArray = (Hazel::VertexArray::Create());

		// 2.�������㻺����
		Hazel::Ref<Hazel::VertexBuffer> squareVB;
		squareVB = (Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		// 2.1���ö��㻺��������
		squareVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		// 3.��������
		uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> squareIB;
		squareIB = (Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_SquareVertexArray->SetIndexBuffer(squareIB);

		// 4.��ɫ��
		std::string squareShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_TexCoord;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec2 v_TexCoord;

			void main(){
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}			
		)";
		std::string squareShaderfragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec2 v_TexCoord;
			
			uniform vec3 u_Color;
				
			void main(){
				color = vec4(v_TexCoord, 0.0f, 1.0f);	
			}			
		)";
		//m_SquareShader.reset(Hazel::Shader::Create(squareShaderVertexSrc, squareShaderfragmentSrc));
		m_SquareShader = Hazel::Shader::Create("squareTexColorShader", squareShaderVertexSrc, squareShaderfragmentSrc);

		// ������Ⱦ�����ε�����
		float squareTexCoordVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		// 1.������������
		//m_SquareTexCoordVertexArray.reset(Hazel::VertexArray::Create());
		m_SquareTexCoordVertexArray = (Hazel::VertexArray::Create());

		// 2.�������㻺����
		Hazel::Ref<Hazel::VertexBuffer> squareTexCoordVB;
		squareTexCoordVB = (Hazel::VertexBuffer::Create(squareTexCoordVertices, sizeof(squareTexCoordVertices)));

		// 2.1���ö��㻺��������
		squareTexCoordVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_SquareTexCoordVertexArray->AddVertexBuffer(squareTexCoordVB);

		// 3.��������
		uint32_t squareTexCoordIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> squareCoordIB;
		squareCoordIB = (Hazel::IndexBuffer::Create(squareTexCoordIndices, sizeof(squareTexCoordIndices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_SquareTexCoordVertexArray->SetIndexBuffer(squareCoordIB);

		// 4.��ɫ��
		//std::string squareTexCoordShaderVertexSrc = R"(
		//)";
		//std::string squareTexCoordShaderfragmentSrc = R"(
		//)";
		//m_SquareTexCoordShader.reset(Hazel::Shader::Create(squareTexCoordShaderVertexSrc, squareTexCoordShaderfragmentSrc));
		//m_SquareTexCoordShader.reset(Hazel::Shader::Create("assets/shaders/Texture.glsl"));
		//m_SquareTexCoordShader = (Hazel::Shader::Create("assets/shaders/Texture.glsl"));
		auto m_SquareTexCoordShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");
		// ֻ��󶨺��ϴ�һ�Σ����Է�������
		m_SquareTexture = Hazel::Texture2D::Create("assets/textures/Checkerboard.png"); // Create���ص���shared_ptr������ֻ��Ҫ��ֵ=
		m_SquareBlendTexture = Hazel::Texture2D::Create("assets/textures/ChernoLogo.png"); // Create���ص���shared_ptr������ֻ��Ҫ��ֵ=
		//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_SquareTexCoordShader)->Bind();
		/*
			��fragment��u_TextureҪ�����������Ϊ0
			��Ϊ����OnUpdate�Ĵ��룬��m_SquareTexture->Bind,������m_SquareTexture��m_RenderID����OpenGL��0���ϣ�
		*/
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_SquareTexCoordShader)->UploadUniformInt("u_Texture", 0);
	}
	void OnUpdate(Hazel::Timestep ts) override {
		//HZ_TRACE("DeltaTime:{0}, millionTime({1})", ts, ts.GetMilliseconds());
		// ��ѯ
		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB)) {
			HZ_TRACE("Tab key is pressed!(POLL)");
		}

		// jkl����������������
		if (Hazel::Input::IsKeyPressed(HZ_KEY_I)) {
			m_flatPosition.y += m_flatMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_K)) {
			m_flatPosition.y -= m_flatMoveSpeed * ts;
		}
		if (Hazel::Input::IsKeyPressed(HZ_KEY_J)) {
			m_flatPosition.x -= m_flatMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_L)) {
			m_flatPosition.x += m_flatMoveSpeed * ts;
		}

		m_CameraController.OnUpdate(ts);

		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::Clear();

		Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

		// ��shader���ȡshader
		auto m_SquareTexCoordShader = m_ShaderLibrary.Get("Texture");

		// 0.�������������
		m_SquareTexture->Bind();
		glm::mat4 squareTexCoordtransfrom = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f });
		Hazel::Renderer::Submit(m_SquareTexCoordShader, m_SquareTexCoordVertexArray, squareTexCoordtransfrom);

		// ��ϵĴ������������
		m_SquareBlendTexture->Bind();
		glm::mat4 squareTexCoordBlendtransfrom = glm::translate(glm::mat4(1.0f), { 0.25f, -0.25f, 0.0f });
		Hazel::Renderer::Submit(m_SquareTexCoordShader, m_SquareTexCoordVertexArray, squareTexCoordBlendtransfrom);

		// 1.��������ɫ��������
		/*glm::mat4 squaretransfrom = glm::translate(glm::mat4(1.0f), { -0.5f, 0.0f, 0.0f });
		Hazel::Renderer::Submit(m_SquareShader, m_SquareVertexArray, squaretransfrom);*/

		// 2.��Ⱦһ��������
		glm::mat4 flattransfrom = glm::translate(glm::mat4(1.0f), m_flatPosition);

		// ������һ�������ε���ɫ��ͨ��imgui������
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		// ����
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), { 0.05f, 0.05f, 0.05f });
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				glm::vec3 pos(i * 0.08f, j * 0.08f, 0.0f);
				glm::mat4 smallsqtransfrom = glm::translate(flattransfrom, pos) * scale;
				Hazel::Renderer::Submit(m_FlatShader, m_FlatVertexArray, smallsqtransfrom);
			}
		}

		Hazel::Renderer::EndScene();
	}
	void OnEvent(Hazel::Event& event) override {
		// �¼�
		m_CameraController.OnEvent(event);
	}

	virtual void OnImgGuiRender()override {
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
private:
	// shader��
	Hazel::ShaderLibrary m_ShaderLibrary;

	// �����
	//Hazel::Ref<Hazel::Shader> m_SquareTexCoordShader;				// shader�� ָ��
	Hazel::Ref<Hazel::VertexArray> m_SquareTexCoordVertexArray;		// ���������� ָ��
	Hazel::Ref<Hazel::Texture2D> m_SquareTexture;		// ����

	// �����Ҫ�õ�����
	Hazel::Ref<Hazel::Texture2D> m_SquareBlendTexture;		// ����

	// ������ɫ��
	Hazel::Ref<Hazel::Shader> m_SquareShader;				// shader�� ָ��
	Hazel::Ref<Hazel::VertexArray> m_SquareVertexArray;		// ���������� ָ��

	// �����
	Hazel::Ref<Hazel::Shader> m_FlatShader;			// shader�� ָ��
	Hazel::Ref<Hazel::VertexArray> m_FlatVertexArray;			// ���������� ָ��

	Hazel::OrthographicCameraController m_CameraController;

	// Ϊ����ƶ���ת������
	// flat��������������
	glm::vec3 m_flatPosition = { 0.7f, 0.7f, 0.0f };
	float m_flatMoveSpeed = 5.0f;

	// ���ε���ɫ
	glm::vec3 m_SquareColor = { 1.0f, 1.0f, 0.0f };
};

class Sandbox : public Hazel::Application {
public:
	Sandbox() {
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
		//std::cout << (-1000 % 1000) << std::endl;
	}
	~Sandbox() {
	}

};
Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
