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
		//GameEngineEditor(ApplicationCommandLineArgs args)
		//	: Application("GameEngine Editor", args)
		GameEngineEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{
			PushLayer(new EditorLayer());
		}
		~GameEngineEditor() {
		}

	};
	// 定义entryPoint的main函数
	Application* CreateApplication(ApplicationCommandLineArgs args) {
		ApplicationSpecification spec;
		spec.Name = "GameEngine Editor";
		spec.CommandLineArgs = args;

		return new GameEngineEditor(spec);
	}
}
