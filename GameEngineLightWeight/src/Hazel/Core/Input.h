#pragma once

#include "Hazel/Core/Core.h"

namespace Hazel {
	class HAZEL_API Input {
	public:
		virtual ~Input() = default;
		 static bool IsKeyPressed(int keycode);

		 static bool IsMouseButtonPressed(int button);
		 static std::pair<float, float> GetMousePosition();
		 static float GetMouseX();
		 static float GetMouseY();
	};
}