#pragma once

#include "entt.hpp"
#include "Entity.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Renderer/EditorCamera.h"

// 不包含box2d的头文件，以便编辑器项目也包含
class b2World;
namespace Hazel {
	class Entity;
	class Scene
	{
	public:
		Scene();
		~Scene();
		
		// 复制场景
		static Ref<Scene> Copy(Ref<Scene> other);

		// 复制实体
		void DuplicateEntity(Entity entity);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		// 创建物理世界环境
		void OnRuntimeStart();
		// 停止
		void OnRuntimeStop();

		void OnUpdateRuntime(Timestep ts);
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
	private:
		entt::registry m_Registry;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

		// 当前场景的路径
		std::string filepath;

		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneSerializer;
	};

}
