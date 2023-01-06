#include <Hazel.h>
#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>

class ExampleLayer :public Hazel::Layer {
public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f) ,m_CameraPosition(0.0f)
	{
		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};
		// 1.������������
		m_VertexArray.reset(Hazel::VertexArray::Create());

		// 2.�������㻺����
		std::shared_ptr<Hazel::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));

		// 2.1���ö��㻺��������
		Hazel::BufferLayout layout = {
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float4, "a_Color"}
		};
		vertexBuffer->SetLayout(layout);

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		// 3.��������
		uint32_t indices[3] = { 0, 1, 2 };

		std::shared_ptr<Hazel::IndexBuffer> indexBuffer;
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// 4.��ɫ��
		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main(){
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}			
		)";
		std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main(){
				color = vec4(v_Position * 0.5 + 0.5, 1.0);	
				color = v_Color;
			}			
		)";
		m_Shader.reset(new Hazel::Shader(vertexSrc, fragmentSrc));

		// �������ݣ���Ⱦ������
		float squareVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		// 1.������������
		m_SquareVA.reset(Hazel::VertexArray::Create());

		// 2.�������㻺����
		std::shared_ptr<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		// 2.1���ö��㻺��������
		squareVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"}
			});

		// 1.1����������Ӷ��㻺��������������������������ò���
		m_SquareVA->AddVertexBuffer(squareVB);

		// 3.��������
		uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };

		std::shared_ptr<Hazel::IndexBuffer> squareIB;
		squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		// 1.2����������������������
		m_SquareVA->SetIndexBuffer(squareIB);

		// 4.��ɫ��
		std::string blueShaderVertexSrc = R"(
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
		std::string blueShaderfragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main(){
				color = vec4(0.2, 0.3, 0.8, 1.0);	
			}			
		)";
		m_BlueShader.reset(new Hazel::Shader(blueShaderVertexSrc, blueShaderfragmentSrc));
	}
	void OnUpdate() override {
		// ��ѯ
		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB)) {
			HZ_TRACE("Tab key is pressed!(POLL)");
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_UP)) {
			m_CameraPosition.y += m_CameraMoveSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_DOWN)) {
			m_CameraPosition.y -= m_CameraMoveSpeed;
		}
		if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT)) {
			m_CameraPosition.x -= m_CameraMoveSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_RIGHT)) {
			m_CameraPosition.x += m_CameraMoveSpeed;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_A)) {
			m_CameraRotation += m_CameraRotationSpeed; // ע����+
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)) {
			m_CameraRotation -= m_CameraRotationSpeed;
		}
		// jkl����������������
		if (Hazel::Input::IsKeyPressed(HZ_KEY_I)) {
			m_SquarePosition.y += m_SquareMoveSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_K)) {
			m_SquarePosition.y -= m_SquareMoveSpeed;
		}
		if (Hazel::Input::IsKeyPressed(HZ_KEY_J)) {
			m_SquarePosition.x -= m_SquareMoveSpeed;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_L)) {
			m_SquarePosition.x += m_SquareMoveSpeed;
		}

		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);
		Hazel::Renderer::BeginScene(m_Camera);

		// ������
		glm::mat4 sqtransfrom = glm::translate(glm::mat4(1.0f), m_SquarePosition);
		Hazel::Renderer::Submit(m_BlueShader, m_SquareVA, sqtransfrom);

		// ��Ⱦһ��������
		// ����
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), {0.05f, 0.05f, 0.05f});
		for (int i = 0; i < 20; i++) {
			for (int j = 0; j < 20; j++) {
				glm::mat4 smallsqtransfrom = glm::translate(glm::mat4(1.0f), { i * 0.08f, j * 0.08f, 0.0f }) * scale;
				Hazel::Renderer::Submit(m_BlueShader, m_SquareVA, smallsqtransfrom);
			}
		}

		// ������
		Hazel::Renderer::Submit(m_Shader, m_VertexArray);

		Hazel::Renderer::EndScene();

	}
	void OnEvent(Hazel::Event& event) override {
		// �¼�
		//if (event.GetEventType() == Hazel::EventType::KeyPressed) {
		//	Hazel::KeyPressedEvent& e = (Hazel::KeyPressedEvent&)event;
		//	if (e.GetKeyCode() == HZ_KEY_TAB) {
		//		HZ_TRACE("Tab key is pressed!(EVENT)");
		//	}
		//	HZ_TRACE("{0}", (char)e.GetKeyCode());
		//}
		// ���¼�����ƶ������
		//Hazel::EventDispatcher dispatcher(event);
		//dispatcher.Dispatch<Hazel::KeyPressedEvent>(HZ_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}
	// �¼�ִ�е�func
	bool OnKeyPressedEvent(Hazel::KeyPressedEvent& event) {
		//if (event.GetKeyCode() == HZ_KEY_UP) {
		//	m_CameraPosition.y += m_CameraMoveSpeed;
		//}
		//else if (event.GetKeyCode() == HZ_KEY_DOWN) {
		//	m_CameraPosition.y -= m_CameraMoveSpeed;
		//}
		//if (event.GetKeyCode() == HZ_KEY_LEFT) {
		//	m_CameraPosition.x -= m_CameraMoveSpeed;
		//}
		//else if (event.GetKeyCode() == HZ_KEY_RIGHT) {
		//	m_CameraPosition.x += m_CameraMoveSpeed;
		//}

		//if (event.GetKeyCode() == HZ_KEY_A) {
		//	m_CameraRotation -= m_CameraRotationSpeed;
		//}
		//else if (event.GetKeyCode() == HZ_KEY_D) {
		//	m_CameraRotation += m_CameraRotationSpeed;
		//}
		return false;
	}

	virtual void OnImgGuiRender()override {
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}
private:
	std::shared_ptr<Hazel::Shader> m_Shader;				// shader�� ָ��
	std::shared_ptr<Hazel::VertexArray> m_VertexArray;		// ���������� ָ��

	std::shared_ptr<Hazel::Shader> m_BlueShader;			// shader�� ָ��
	std::shared_ptr<Hazel::VertexArray> m_SquareVA;			// ���������� ָ��

	Hazel::OrthographicCamera m_Camera;

	// Ϊ����ƶ���ת������
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 0.05f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 1.0f;

	// ���ε�������������
	glm::vec3 m_SquarePosition = { -1.0f, -1.0f, -1.0f };
	float m_SquareMoveSpeed = 0.05f;
};

class Sandbox : public Hazel::Application {
public:
	Sandbox() {
		PushLayer(new ExampleLayer());
		//PushOverlay(new Hazel::ImGuiLayer());
	}
	~Sandbox() {
	}

};
Hazel::Application* Hazel::CreateApplication() {
	return new Sandbox();
}
//int main(int argc, char** argv) {
//	auto app = new Sandbox();
//	app->Run();
//	delete app;
//}