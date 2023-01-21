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
        // ���Ĭ�����
        Entity entity = { m_Registry.create(),this };
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }
    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }
    void Scene::OnUpdate(Timestep ts)
    {
        // �������е�ʱ����½ű���
        {   //  [=]����ʽֵ���񣬲���ts
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
                if (!nsc.Instance) {
                    nsc.Instance = nsc.InstantiateScript();
                    nsc.Instance->m_Entity = Entity{ entity, this };
                    // ִ��CameraController�ű���OnCreate���������麯��ָ��
                    nsc.Instance->OnCreate();
                }
                // ִ��CameraController�ű���OnUpdate����
                nsc.Instance->OnUpdate(ts);
            });
        }
        
        // ��ȡ��������������һ�ȡ���������λ�ã���������ͶӰ����projection
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
                // get���ص�tuple�ﱾ������
                auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawQuad(tfc.GetTransform(), sprite.Color);
            }
            if (group.size() <= 0) {
                Renderer2D::Shutdown();
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
    // ģ���ඨ����cpp��
    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        // ��̬���ԣ�false�������ڱ���ǰ�ͻ�ִ�У� ���Ǳ��������ﲻ�ᱨ��˵����δ��벻�����ɡ���
        // ���Ҵ��˶ϵ㣬Ҳ���У�֤����δ���ֻ���������ðɡ�
        static_assert(false);
    }
    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        entity.GetComponent<TransformComponent>().Translation = { 0, 0, 5.0f };
        component.camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }
    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
    {
    }
}