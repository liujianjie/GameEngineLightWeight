#pragma once

#include "Core.h"
namespace Hazel {
	class HAZEL_API Application
	{
	public:
		Application();
		~Application();

		void Run();
	};

	// 将在客户端被定义
	//Application* CreateApplication();
}

