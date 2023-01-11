#include "hzpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <gl/GL.h>

namespace Hazel {

	Hazel::OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void Hazel::OpenGLContext::Init()
	{
		HZ_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HZ_CORE_ASSERT(status, "Failed to initialize Glad!");

		HZ_CORE_INFO("OpenGL Info:");
		std::cout << "Vendor " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "Renderer " << glGetString(GL_RENDERER) << std::endl;
		std::cout << "Version " << glGetString(GL_VERSION) << std::endl;
		//HZ_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		//HZ_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		//HZ_CORE_INFO("  Version: {0}", glGetString(GL_VERSION));
	}

	void Hazel::OpenGLContext::SwapBuffers()
	{
		HZ_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}
}