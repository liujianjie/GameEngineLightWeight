#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "entt.hpp"
#include "Entity.h"

// ������box2d��ͷ�ļ����Ա�༭����ĿҲ����
class b2World;
namespace Hazel {
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		// ���Ƴ���
		static Ref<Scene> Copy(Ref<Scene> other);

		// ����ʵ��
		void DuplicateEntity(Entity entity);

		Entity GetEntityByUUID(UUID uuid);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		// �����������绷��
		void OnRuntimeStart();
		// ֹͣ
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();

		template<typename... Components>
		auto GetAllEntitiesWith() {
			return m_Registry.view<Components...>();
		}

		std::string GetCurFilePath() { return filepath; }
		void SetCurFilePath(const std::string& path) { filepath = path; }
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void RenderScene(EditorCamera& camera);
	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		// ��ǰ������·��
		std::string filepath;
		// map��uuid->entity
		std::unordered_map<UUID, entt::entity> m_EntityMap;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};

}
