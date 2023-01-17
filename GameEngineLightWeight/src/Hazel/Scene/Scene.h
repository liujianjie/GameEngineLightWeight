#pragma once

#include "entt.hpp"

#include "Hazel/Core/Timestep.h"

namespace Hazel {
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		entt::entity CreateEnitty();

		// TEMP
		entt::registry& Reg() { return m_Registry; }

		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;
	};
}
