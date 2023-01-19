#pragma once

#include "Scene.h"
#include "entt.hpp"
namespace Hazel {
	class Scene;
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			HZ_CORE_ASSERT(!HasComponent<T>(), "实体已经存在这个组件");
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}
		template<typename T>
		T& GetComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "实体不存在这个组件");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}
		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}
		template<typename T>
		void RemoveComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "实体不存在这个组件");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		operator bool() const { return m_EntityHandle != entt::null; }

		operator uint32_t() { return (uint32_t)m_EntityHandle; }
		bool operator==(const Entity& other)const {
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}
		bool operator!=(const Entity& other)const {
			return !(*this == other); // 调用上面的==运算函数
		}
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}

