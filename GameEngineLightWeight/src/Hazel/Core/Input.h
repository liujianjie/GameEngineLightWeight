#pragma once

#include "Hazel/Core/Core.h"
#include "Hazel/Core/KeyCodes.h"

namespace Hazel {
	class HAZEL_API Input {
	public:
		virtual ~Input() = default;
		 static bool IsKeyPressed(KeyCode key);

		 static bool IsMouseButtonPressed(int button);
		 static std::pair<float, float> GetMousePosition();
		 static float GetMouseX();
		 static float GetMouseY();
	};
}