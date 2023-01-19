#pragma once
#include "Entity.h"
namespace Hazel {
	class ScriptableEntity {
	public:
		//virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent() {
			return m_Entity.GetComponent<T>(); // 根据Entity类找到关联的组件
		}
	//protected:
	//	virtual void OnCreate() {}
	//	virtual void OnDestroy() {}
	//	virtual void OnUpdate(Timestep ts) {}
	private:
		Entity m_Entity; 
		friend class Scene;// 为了在scene中设置m_Entity
	};
}