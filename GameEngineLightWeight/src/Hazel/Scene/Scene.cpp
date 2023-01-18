#include "hzpch.h"
#include "Scene.h"

#include "Components.h"
#include "Hazel/Renderer/Renderer2D.h"
#include <glm/glm.hpp>

namespace Hazel {
    static void DoMath(const glm::mat4& transform) {
    }
    static void OnTransformConstruct(entt::registry& registry, entt::entity entity) {
    }
    Scene::Scene()
    {
        m_Registry.create(); // ��ֵ+1
#if ENTT_EXAMPLE_CODE
        TransformComponent transform;
        // ����
        DoMath(transform.Transform);
        DoMath(transform); // ��Ϊ������ת������
        // ��û������ת������
        DoMath(*(glm::mat4*)&transform);

        // 1.����һ��ʵ��,�������� id
        entt::entity entity = m_Registry.create(); // ��uint32_t
        // 2.ʵ�����TransformComponent���.arg1:id��arg2��glm��ת��ΪTransformComponent
        m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));
        // 3.�ж�ʵ���Ƿ���ŶTransformComponent���
        //if (m_Registry.has<TransformComponent>(entity))  // ���뱨��֪��Ϊʲô
        // 4.��ȡʵ���TransformComponent���,����id
        TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
        // 5.m_Registry��ע�������TransformComponent���
        auto view = m_Registry.view<TransformComponent>();
        for (auto entity : view) {
            TransformComponent& transform1 = m_Registry.get<TransformComponent>(entity); // view��m_Registry��������
            TransformComponent& transform2 = view.get<TransformComponent>(entity);
        }
        // 6.m_Registry��ȡ����Transform�������Mesh�����ʵ��
        auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
        for (auto entity : group) {
            auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
            //Renderer::Submit(mesh, transform);
        }
        // 7.���õ�ʵ�����TransformComponent���ʱִ��OnTransformConstruct����
        m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
#endif
    }
    Scene::~Scene()
    {
    }
    Entity Scene::CreateEntity(std::string name)
    {
        // ���Ĭ�����
        Entity entity = { m_Registry.create(),this };
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }
    void Scene::OnUpdate(Timestep ts)
    {
        // ��ȡ��������������һ�ȡ���������λ�ã���������ͶӰ����projection
        Camera* mainCamera = nullptr;
        glm::mat4* cameraTransform = nullptr;
        {
            auto group = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : group) {
                auto& [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

                if (camera.primary) {
                    mainCamera = &camera.camera;
                    cameraTransform = &transform.Transform;
                }
            }
        }
        if (mainCamera) {
            Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group) {
                auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawQuad(transform, sprite.Color);
            }
            Renderer2D::EndScene();
        }
    }
    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view) {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.fixedAspectRatio) {
                cameraComponent.camera.SetViewportSize(width, height);
            }
        }
    }
}