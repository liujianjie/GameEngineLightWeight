#pragma once
#include "Entity.h"
namespace Hazel {
	class ScriptableEntity {
	public:
		//virtual ~ScriptableEntity() {}

		template<typename T>
		T& GetComponent() {
			return m_Entity.GetComponent<T>(); // ����Entity���ҵ����������
		}
	//protected:
	//	virtual void OnCreate() {}
	//	virtual void OnDestroy() {}
	//	virtual void OnUpdate(Timestep ts) {}
	private:
		Entity m_Entity; 
		friend class Scene;// Ϊ����scene������m_Entity
	};
}