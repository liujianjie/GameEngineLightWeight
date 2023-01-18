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
        m_Registry.create(); // 让值+1
#if ENTT_EXAMPLE_CODE
        TransformComponent transform;
        // 测试
        DoMath(transform.Transform);
        DoMath(transform); // 因为有类型转换函数
        // 若没有类型转换函数
        DoMath(*(glm::mat4*)&transform);

        // 1.创建一个实体,返回整数 id
        entt::entity entity = m_Registry.create(); // 是uint32_t
        // 2.实体放入TransformComponent组件.arg1:id，arg2：glm会转换为TransformComponent
        m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));
        // 3.判断实体是否有哦TransformComponent组件
        //if (m_Registry.has<TransformComponent>(entity))  // 代码报错不知道为什么
        // 4.获取实体的TransformComponent组件,根据id
        TransformComponent& transform = m_Registry.get<TransformComponent>(entity);
        // 5.m_Registry已注册的所有TransformComponent组件
        auto view = m_Registry.view<TransformComponent>();
        for (auto entity : view) {
            TransformComponent& transform1 = m_Registry.get<TransformComponent>(entity); // view、m_Registry两个都行
            TransformComponent& transform2 = view.get<TransformComponent>(entity);
        }
        // 6.m_Registry获取既有Transform组件又有Mesh组件的实体
        auto group = m_Registry.group<TransformComponent>(entt::get<MeshComponent>);
        for (auto entity : group) {
            auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
            //Renderer::Submit(mesh, transform);
        }
        // 7.设置当实体添加TransformComponent组件时执行OnTransformConstruct方法
        m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
#endif
    }
    Scene::~Scene()
    {
    }
    Entity Scene::CreateEntity(std::string name)
    {
        // 添加默认组件
        Entity entity = { m_Registry.create(),this };
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }
    void Scene::OnUpdate(Timestep ts)
    {
        // 获取到主摄像机，并且获取到摄像机的位置，用来计算投影矩阵projection
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