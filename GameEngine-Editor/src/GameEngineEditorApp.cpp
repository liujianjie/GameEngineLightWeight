#include <Hazel.h>
#include "imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Hazel/Core/EntryPoint.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EditorLayer.h"

namespace Hazel {
	class GameEngineEditor : public Application {
	public:
		GameEngineEditor(ApplicationCommandLineArgs args)
			: Application("GameEngine Editor", args)
		{
			PushLayer(new EditorLayer());
		}
		~GameEngineEditor() {
		}

	};
	// ´Ómain´«Èë
	Application* CreateApplication(ApplicationCommandLineArgs args) {
		return new GameEngineEditor(args);
	}

}
