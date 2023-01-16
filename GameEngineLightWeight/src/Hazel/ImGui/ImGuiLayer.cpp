#include "hzpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"

#include "examples/imgui_impl_opengl3.h"
#include "examples/imgui_impl_glfw.h"

#include "Hazel/Core/Application.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
namespace Hazel {
	ImGuiLayer::ImGuiLayer():Layer("ImGuiLayer") {

	}

	ImGuiLayer::~ImGuiLayer() {

	}

	void ImGuiLayer::OnAttach() {
		HZ_PROFILE_FUNCTION();
		//std::cout << "调用ImGuiLayer的OnAttach了吗" << std::endl;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

	}
	void ImGuiLayer::OnDetach() {
		HZ_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	void ImGuiLayer::OnEvent(Event& e) {
		/*
		m_BlockEvents：false-> 窗口不阻塞事件，viewport面板能接收滚轮事件
		m_BlockEvents：true->  窗口  阻塞事件，viewport面板不能接收滚轮事件
		*/
		if (m_BlockEvents) {
			/*
				imgui窗口会阻塞鼠标滚轮事件，即ImGuiLayer窗口处理了鼠标滚轮事件，不会传入给下一级的OnEvent。
				因为Application的onevent处理了窗口滚动，就会跳出
				for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
					if (e.Handled)
						break;
					(*--it)->OnEvent(e);
				}
			*/
			ImGuiIO& io = ImGui::GetIO();
			e.Handled |= e.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse; // e.Handled = e.handled | true & true; 结果为true，所以上述的application 的 event会跳出
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}
	void ImGuiLayer::Begin()
	{
		HZ_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		HZ_PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		// 渲染
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);

		}
	}
	void ImGuiLayer::OnImGuiRender()
	{
		//static bool show = true;
		//ImGui::ShowDemoWindow(&show);
	}
	
}