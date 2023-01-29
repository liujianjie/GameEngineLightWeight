#pragma once
#include "Hazel/Core/Core.h"
#include "Hazel/Core/Application.h"

#ifdef  HZ_PLATFORM_WINDOWS

// 表示在其他地方定义
extern Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv) {
	Hazel::Log::Init();

	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	auto app = Hazel::CreateApplication({argc, argv});
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	app->Run();
	HZ_PROFILE_END_SESSION();

	HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Shutdown.json");
	delete app;
	HZ_PROFILE_END_SESSION();
}

#endif //  HZ_PLATFORM_WINDOWS
