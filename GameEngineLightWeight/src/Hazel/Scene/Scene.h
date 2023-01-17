#pragma once

#include "entt.hpp"
#include "Entity.h"
#include "Hazel/Core/Timestep.h"

namespace Hazel {
	class Entity;
	class Scene
	{
		friend class Entity;
	public:
		Scene();
		~Scene();
		
		Entity CreateEnitty(std::string name);

		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;
	};
}
