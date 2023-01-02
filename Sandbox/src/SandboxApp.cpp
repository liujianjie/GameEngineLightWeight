#include <Hazel.h>
#include "imgui/imgui.h"

class ExampleLayer :public Hazel::Layer {
public:
	ExampleLayer() : Layer("Example") {

	}
	void OnUpdate() override {
		// ÂÖÑ¯
		if (Hazel::Input::IsKeyPressed(HZ_KEY_TAB)) {
			HZ_TRACE("Tab key is pressed!(POLL)");
		}
	}
	void OnEvent(Hazel::Event& event) override {
		// ÊÂ¼þ
		if (event.GetEventType() == Hazel::EventType::KeyPressed) {
			Hazel::KeyPressedEvent& e = (Hazel::KeyPressedEvent&)event;
			if (e.GetKeyCode() == HZ_KEY_TAB) {
				HZ_TRACE("Tab key is pressed!(EVENT)");
			}
			HZ_TRACE("{0}", (char)e.GetKeyCode());
		}
	}
	virtual void OnImgGuiRender()override {
		//ImGui::Begin("Test");
		//ImGui::Text("Hello World");
		//ImGui::End();
	}
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