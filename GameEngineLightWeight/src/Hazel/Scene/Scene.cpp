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
        // 引擎运行的时候更新脚本。
        {   //  [=]是隐式值捕获，捕获ts
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
                if (!nsc.Instance) {
                    nsc.Instance = nsc.InstantiateScript();
                    nsc.Instance->m_Entity = Entity{ entity, this };
                    // 执行CameraController脚本的OnCreate函数，由虚函数指定
                    nsc.Instance->OnCreate();
                }
                // 执行CameraController脚本的OnUpdate函数
                nsc.Instance->OnUpdate(ts);
            });
        }
        
        // 获取到主摄像机，并且获取到摄像机的位置，用来计算投影矩阵projection
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto group = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : group) {
                auto [tfc, camera] = group.get<TransformComponent, CameraComponent>(entity);

                if (camera.primary) {
                    mainCamera = &camera.camera;
                    cameraTransform = tfc.GetTransform();
                }
            }
        }
        if (mainCamera) {
            Renderer2D::BeginScene(*mainCamera, cameraTransform);
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group) {
                // get返回的tuple里本是引用
                auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawQuad(tfc.GetTransform(), sprite.Color);
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