#include <Hazel.h>
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Core/EntryPoint.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

class GameEngineEditor : public Hazel::Application {
public:
	GameEngineEditor()
		: Application("GameEngine Editor")
	{
		PushLayer(new EditorLayer());
	}
	~GameEngineEditor() {
	}

};
Hazel::Application* Hazel::CreateApplication() {
	return new GameEngineEditor();
}
