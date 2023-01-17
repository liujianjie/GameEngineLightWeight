#include "hzpch.h"
#include "Scene.h"
#include "glm/glm.hpp"
namespace Hazel {
    static void DoMath(const glm::mat4& transform) {

    }
    static void OnTransformConstruct(entt::registry& registry, entt::entity entity) {

    }
    Scene::Scene()
    {
        struct TransformComponent { // ���ü̳�Component
            glm::mat4 Transform;
            TransformComponent() = default;
            TransformComponent(const TransformComponent&) = default;
            TransformComponent(const glm::mat4& transform)
                : Transform(transform){}
            operator const glm::mat4& () { return Transform; }
        };
        struct MeshComponent {
            glm::mat4 Transform;
            MeshComponent() = default;
            MeshComponent(const MeshComponent&) = default;
            MeshComponent(const glm::mat4& transform)
                : Transform(transform) {}
            operator const glm::mat4& () { return Transform; }
        };
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
            auto&[transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
            //Renderer::Submit(mesh, transform);
        }
        // 7.���õ�ʵ�����TransformComponent���ʱִ��OnTransformConstruct����
        m_Registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
    }
    Scene::~Scene()
    {
    }
}