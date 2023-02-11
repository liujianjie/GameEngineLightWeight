#pragma once

#include "Hazel/Core/UUID.h"
#include "Components.h"
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
			HZ_CORE_ASSERT(!HasComponent<T>(), "ʵ���Ѿ�����������");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			// ����������ʱִ�еķ���
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}
		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) {
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);// ����������ʱִ�еķ���
			return component;
		}
		template<typename T>
		T& GetComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "ʵ�岻����������");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}
		template<typename T>
		T& GetComponent() const{
			HZ_CORE_ASSERT(HasComponent<T>(), "ʵ�岻����������");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}
		template<typename T>
		bool HasComponent() {
			return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
		}
		template<typename T>
		void RemoveComponent() {
			HZ_CORE_ASSERT(HasComponent<T>(), "ʵ�岻����������");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}
		// ��ȡuuid
		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		UUID GetUUID() const { return GetComponent<IDComponent>().ID; }
		// ��ȡtag����
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		operator bool() const { 
			return m_EntityHandle != entt::null; 
		}
		operator uint32_t() const{ return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }
		bool operator==(const Entity& other)const {
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}
		bool operator!=(const Entity& other)const {
			return !(*this == other); // ���������==���㺯��
		}
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}

