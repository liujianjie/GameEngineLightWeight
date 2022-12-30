#include <Hazel.h>

class Sandbox : public Hazel::Application {
public:
	Sandbox() {

	}
	~Sandbox() {

	}

};
//Hazel::Application* Hazel::CreateApplication() {
//	return new Sandbox();
//}
int main(int argc, char** argv) {
	auto app = new Sandbox();
	app->Run();
	delete app;
}