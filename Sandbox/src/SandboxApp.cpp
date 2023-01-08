#include <Hazel.h>
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer :public Hazel::Layer {
public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f) ,m_CameraPosition(0.0f)
	{
		// 渲染网格 flat
		float flatVertices[3 * 4] = {
			-0.75f, -0.75f, 0.0f,
			 0.75f, -0.75f, 0.0f,
			 0.75f,  0.75f, 0.0f,
			-0.75f,  0.75f, 0.0f
		};
		// 1.创建顶点数组
		m_FlatVertexArray.reset(Hazel::VertexArray::Create());

		// 2.创建顶点缓冲区
		Hazel::Ref<Hazel::VertexBuffer> flatVB;
		flatVB.reset(Hazel::VertexBuffer::Create(flatVertices, sizeof(flatVertices)));

		// 2.1设置顶点缓冲区布局
		flatVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"}
			});

		// 1.1顶点数组添加顶点缓冲区，并且在这个缓冲区中设置布局
		m_FlatVertexArray->AddVertexBuffer(flatVB);

		// 3.索引缓冲
		uint32_t flatIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> flatIB;
		flatIB.reset(Hazel::IndexBuffer::Create(flatIndices, sizeof(flatIndices) / sizeof(uint32_t)));

		// 1.2顶点数组设置索引缓冲区
		m_FlatVertexArray->SetIndexBuffer(flatIB);

		// 4.着色器
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
		m_FlatShader.reset(Hazel::Shader::Create(flatShaderVertexSrc, flatShaderfragmentSrc));


		// 二、渲染正方形的纹理颜色。这里纹理坐标是基于左下角为中心算的，负的为0，正的为1，比如(-0.5f, 0.5f)的坐标纹理是(0.0f, 1.0f)
		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		// 1.创建顶点数组
		m_SquareVertexArray.reset(Hazel::VertexArray::Create());

		// 2.创建顶点缓冲区
		Hazel::Ref<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));

		// 2.1设置顶点缓冲区布局
		squareVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1顶点数组添加顶点缓冲区，并且在这个缓冲区中设置布局
		m_SquareVertexArray->AddVertexBuffer(squareVB);

		// 3.索引缓冲
		uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> squareIB;
		squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		// 1.2顶点数组设置索引缓冲区
		m_SquareVertexArray->SetIndexBuffer(squareIB);

		// 4.着色器
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
		m_SquareShader.reset(Hazel::Shader::Create(squareShaderVertexSrc, squareShaderfragmentSrc));

		// 三、渲染正方形的纹理。
		float squareTexCoordVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};
		// 1.创建顶点数组
		m_SquareTexCoordVertexArray.reset(Hazel::VertexArray::Create());

		// 2.创建顶点缓冲区
		Hazel::Ref<Hazel::VertexBuffer> squareTexCoordVB;
		squareTexCoordVB.reset(Hazel::VertexBuffer::Create(squareTexCoordVertices, sizeof(squareTexCoordVertices)));

		// 2.1设置顶点缓冲区布局
		squareTexCoordVB->SetLayout({
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
			});

		// 1.1顶点数组添加顶点缓冲区，并且在这个缓冲区中设置布局
		m_SquareTexCoordVertexArray->AddVertexBuffer(squareTexCoordVB);

		// 3.索引缓冲
		uint32_t squareTexCoordIndices[] = { 0, 1, 2, 2, 3, 0 };

		Hazel::Ref<Hazel::IndexBuffer> squareCoordIB;
		squareCoordIB.reset(Hazel::IndexBuffer::Create(squareTexCoordIndices, sizeof(squareTexCoordIndices) / sizeof(uint32_t)));

		// 1.2顶点数组设置索引缓冲区
		m_SquareTexCoordVertexArray->SetIndexBuffer(squareCoordIB);

		// 4.着色器
		//std::string squareTexCoordShaderVertexSrc = R"(
		//)";
		//std::string squareTexCoordShaderfragmentSrc = R"(
		//)";
		//m_SquareTexCoordShader.reset(Hazel::Shader::Create(squareTexCoordShaderVertexSrc, squareTexCoordShaderfragmentSrc));
		m_SquareTexCoordShader.reset(Hazel::Shader::Create("asserts/shaders/Texture.glsl"));
		// 只需绑定和上传一次，所以放在这里
		m_SquareTexture = Hazel::Texture2D::Create("asserts/textures/Checkerboard.png"); // Create返回的是shared_ptr，所以只需要赋值=
		m_SquareBlendTexture = Hazel::Texture2D::Create("asserts/textures/ChernoLogo.png"); // Create返回的是shared_ptr，所以只需要赋值=
		//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_SquareTexCoordShader)->Bind();
		/*把fragment的u_Texture要采样的纹理槽为0
		因为下面的代码，把m_SquareTexture->Bind,设置了m_SquareTexture的m_RenderID绑定在OpenGL的0槽上！
		*/
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_SquareTexCoordShader)->UploadUniformInt("u_Texture", 0);
	}
	void OnUpdate(Hazel::Timestep ts) override {
		//HZ_TRACE("DeltaTime:{0}, millionTime({1})", ts, ts.GetMilliseconds());
		// 轮询
		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB)) {
			HZ_TRACE("Tab key is pressed!(POLL)");
		}
		if (Hazel::Input::IsKeyPressed(HZ_KEY_UP)) {
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_DOWN)) {
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;
		}
		if (Hazel::Input::IsKeyPressed(HZ_KEY_LEFT)) {
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_RIGHT)) {
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		}

		if (Hazel::Input::IsKeyPressed(HZ_KEY_A)) {
			m_CameraRotation += m_CameraRotationSpeed * ts; // 注意是+
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)) {
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		}
		// jkl控制物体的世界矩阵
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

		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Hazel::Renderer::BeginScene(m_Camera);

		// 0.带纹理的正方形
		m_SquareTexture->Bind();
		glm::mat4 squareTexCoordtransfrom = glm::translate(glm::mat4(1.0f), { 0.0f, 0.0f, 0.0f });
		Hazel::Renderer::Submit(m_SquareTexCoordShader, m_SquareTexCoordVertexArray, squareTexCoordtransfrom);

		// 混合的带纹理的正方形
		m_SquareBlendTexture->Bind();
		glm::mat4 squareTexCoordBlendtransfrom = glm::translate(glm::mat4(1.0f), { 0.25f, -0.25f, 0.0f });
		Hazel::Renderer::Submit(m_SquareTexCoordShader, m_SquareTexCoordVertexArray, squareTexCoordBlendtransfrom);

		// 1.带纹理颜色的正方形
		/*glm::mat4 squaretransfrom = glm::translate(glm::mat4(1.0f), { -0.5f, 0.0f, 0.0f });
		Hazel::Renderer::Submit(m_SquareShader, m_SquareVertexArray, squaretransfrom);*/

		// 2.渲染一组正方形
		glm::mat4 flattransfrom = glm::translate(glm::mat4(1.0f), m_flatPosition);

		// 设置这一组正方形的颜色，通过imgui来设置
		//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->Bind();
		//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_FlatShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		// 缩放
		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), {0.05f, 0.05f, 0.05f});
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
		// 事件
		//if (event.GetEventType() == Hazel::EventType::KeyPressed) {
		//	Hazel::KeyPressedEvent& e = (Hazel::KeyPressedEvent&)event;
		//	if (e.GetKeyCode() == HZ_KEY_TAB) {
		//		HZ_TRACE("Tab key is pressed!(EVENT)");
		//	}
		//	HZ_TRACE("{0}", (char)e.GetKeyCode());
		//}
	}

	virtual void OnImgGuiRender()override {
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color",glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
private:
	// 纹理的
	Hazel::Ref<Hazel::Shader> m_SquareTexCoordShader;				// shader类 指针
	Hazel::Ref<Hazel::VertexArray> m_SquareTexCoordVertexArray;		// 顶点数组类 指针
	Hazel::Ref<Hazel::Texture2D> m_SquareTexture;		// 纹理

	// 混合需要用的纹理
	Hazel::Ref<Hazel::Texture2D> m_SquareBlendTexture;		// 纹理

	// 纹理颜色的
	Hazel::Ref<Hazel::Shader> m_SquareShader;				// shader类 指针
	Hazel::Ref<Hazel::VertexArray> m_SquareVertexArray;		// 顶点数组类 指针

	// 网格的
	Hazel::Ref<Hazel::Shader> m_FlatShader;			// shader类 指针
	Hazel::Ref<Hazel::VertexArray> m_FlatVertexArray;			// 顶点数组类 指针

	Hazel::OrthographicCamera m_Camera;

	// 为完成移动旋转的属性
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;

	// flat的世界矩阵的属性
	glm::vec3 m_flatPosition = { 0.7f, 0.7f, 0.0f };
	float m_flatMoveSpeed = 5.0f;

	// 矩形的颜色
	glm::vec3 m_SquareColor = { 0.0f, 0.0f, 0.0f };
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
