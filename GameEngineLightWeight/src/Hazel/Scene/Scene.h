#pragma once

#include "entt.hpp"
namespace Hazel {
	class Scene
	{
	public:
		Scene();
		~Scene();
		
	private:
		entt::registry m_Registry;
	};
}
