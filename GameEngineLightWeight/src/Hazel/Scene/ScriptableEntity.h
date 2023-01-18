#pragma once
#include "Entity.h"
namespace Hazel {
	class ScriptableEntity {
	public:
		template<typename T>
		T& GetComponent() {
			return m_Entity.GetComponent<T>(); // 根据Entity类找到关联的组件
		}
	private:
		Entity m_Entity; // 
		friend class Scene;
	};
}