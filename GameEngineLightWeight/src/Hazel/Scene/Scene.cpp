#include "hzpch.h"
#include "Scene.h"

#include "Components.h"
#include "ScriptableEntity.h"

#include "Hazel/Renderer/Renderer2D.h"
#include <glm/glm.hpp>

//#include "SceneCamera.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Hazel {

    // �Զ������������ת��Ϊbox2d������
    static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType) {
        switch (bodyType)
        {
            case Rigidbody2DComponent::BodyType::Static: return b2_staticBody;
            case Rigidbody2DComponent::BodyType::Dynamic: return b2_dynamicBody;
            case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
        }
        HZ_CORE_ASSERT(false, "Unkown body type");
        return b2_staticBody;
    }

    Scene::Scene()
    {
    }
    Scene::~Scene()
    {
    }
    // Ϊ���Ƴ�����ʵ�������ĸ�������
    template<typename Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {
        auto view = src.view<Component>();
        // 2.1�����ɳ�������uuid����ľ�ʵ��
        for (auto e : view) {
            UUID uuid = src.get<IDComponent>(e).ID;
            HZ_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
            // 2.2��** ��ʵ���uuid - map - ��Ӧ��ʵ�� * *
            entt::entity dstEnttID = enttMap.at(uuid);
            // 3.1��ȡ��ʵ������
            auto& component = src.get<Component>(e);
            // 3.2Ȼ����API��** ���ƾ�ʵ����������ʵ��**
            dst.emplace_or_replace<Component>(dstEnttID, component);// ��ӻ��滻������
        }
    }
    // Ϊ����ʵ��ĸ�������
    template<typename Component>
    static void CopyComponentIfExists(Entity dst, Entity src) {
        if (src.HasComponent<Component>()) {
            dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
        }
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        // 1.1�����³���
        Ref<Scene> newScene = CreateRef<Scene>();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        auto& srcSceneRegistry = other->m_Registry;
        auto& dstSceneRegistry = newScene->m_Registry;
        std::unordered_map<UUID, entt::entity> enttMap;

        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e : idView) {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
            // 1.2Ϊ�³��������;ɳ���ͬ����uuid��ʵ��
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            // 1.3����**map���루��ʵ���uuid��Ӧ��ʵ�壩�Ĺ�ϵ**
            enttMap[uuid] = (entt::entity)newEntity;// UUID����Ҫ��ϣ
        }

        // �������������IDcomponent��tagcomponent����CreateEntityWithUUID������
        CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
    }

    void Scene::DuplicateEntity(Entity entity)
    {
        // 1.������ʵ��ͬ������ʵ��
        std::string name = entity.GetName();
        Entity newEntity = CreateEntity(name);
        // 2.�������
        CopyComponentIfExists<TransformComponent>(newEntity, entity);
        CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
        CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
        CopyComponentIfExists<CameraComponent>(newEntity, entity);
        CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
        CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
        CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
        CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);
    }
    Entity Scene::CreateEntity(const std::string& name)
    {
        // �����µ�uuid
        return CreateEntityWithUUID(UUID(), name);
    }
    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        // ���Ĭ�����
        Entity entity = { m_Registry.create(),this };
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<IDComponent>(uuid); // ʹ�ô��ڵ�uuid���������µ�
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }
    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }
    void Scene::OnRuntimeStart()
    {
        // 1.����һ����������
        m_PhysicsWorld = new b2World({0.0f, -9.8f});// �������ٶ�����
        // 1.1Ϊ��ǰ�������о������������ʵ�崴��b2Body
        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view) {
            Entity entity = { e, this };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            // 2.1 ���嶨������ָ����̬���ͺͲ���
            b2BodyDef bodyDef;
            bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;   // ����z����ת
            // 2.2 ��b2BodyDef��������
            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation); // �Ƿ�̶���ת

            rb2d.RuntimeBody = body;

            if (entity.HasComponent<BoxCollider2DComponent>()) {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                // 3.1����Box��Χ��
                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);// ��Χ�и��������size���仯
                // 3.2����fixture��fixture��������İ�Χ��
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = bc2d.Density;
                fixtureDef.friction = bc2d.Friction;
                fixtureDef.restitution = bc2d.Restitution;
                fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                // 3.3���������fixture
                body->CreateFixture(&fixtureDef);
            }
            if (entity.HasComponent<CircleCollider2DComponent>()) {
                auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
                // 3.1����Բ�ΰ�Χ��
                b2CircleShape circleShape;
                circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
                circleShape.m_radius = cc2d.Radius;
                // 3.2����fixture��fixture��������İ�Χ��
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circleShape;
                fixtureDef.density = cc2d.Density;
                fixtureDef.friction = cc2d.Friction;
                fixtureDef.restitution = cc2d.Restitution;
                fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
                // 3.3���������fixture
                body->CreateFixture(&fixtureDef);
            }
        }
    }
    void Scene::OnRuntimeStop()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }
    void Scene::OnUpdateRuntime(Timestep ts)
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
        {
            // ��script�ű�Ӱ��Physics�仯�ٵ�ǰ֡��Ⱦ����
            // �����ٶȣ�ʹ�ø��ٵĵ�������������ܣ���׼ȷ�Ի��ܵ�Ӱ�졣ʹ�ø�������ή�����ܵ������ģ������
            // �е㲻����������˵ɶ��ʱ�䲽���͵���������ȫ�޹ء����������Ӳ���
            // Cherno˵�����ٶȣ���ý���һ�μ���ģ�⡣�������6���Ƕ��ٺ������6����
            const int32_t velocityIterations = 6;// ��Щ����Ӧ���Ƶ��༭��
            const int32_t positionIterations = 2;
            m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

            auto view = m_Registry.view<Rigidbody2DComponent>();
            for (auto e : view) {
                Entity entity = { e, this };
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                // ��ȡ����ģ�����������
                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                // ��������ֵ����ʵ��
                const auto& position = body->GetPosition();
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = body->GetAngle();// ��ȡz��Ƕ�
            }
            // �ű�Ӱ��Pyhsics��������Ⱦ����
        }
        if (mainCamera) {
            Renderer2D::BeginScene(*mainCamera, cameraTransform);
            // �滭sprite
            {
                auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                for (auto entity : group) {
                    // get���ص�tuple�ﱾ������
                    auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                    Renderer2D::DrawSprite(tfc.GetTransform(), sprite, (int)entity);
                }
            }
            // �滭circles
            {
                auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                for (auto entity : view) {
                    // get���ص�tuple�ﱾ������
                    auto [tfc, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                    Renderer2D::DrawCircle(tfc.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
                }
            }
            Renderer2D::EndScene();
        }
    }
    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
        Renderer2D::BeginScene(camera);
        // �滭sprite
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group) {
                // get���ص�tuple�ﱾ������
                auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawSprite(tfc.GetTransform(), sprite, (int)entity);
                //Renderer2D::DrawRect(tfc.GetTransform(), glm::vec4(1.0f), (int)entity);
            }
        }
        // �滭circles
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view) {
                // get���ص�tuple�ﱾ������
                auto [tfc, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);
                Renderer2D::DrawCircle(tfc.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
            }
        }
        //Renderer2D::DrawLine(glm::vec3(2.0f), glm::vec3(5.0f), glm::vec4(1, 0, 1, 1));
        //Renderer2D::DrawRect(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec4(1, 0, 1, 1));
        Renderer2D::EndScene();
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
    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view) {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.primary) {
                return Entity{ entity, this };
            }
        }
        return {};
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
    void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        // �����͸�������������z=0������������
        if (component.camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective
            && entity.GetComponent<TransformComponent>().Translation.z == 0.0f) {
            entity.GetComponent<TransformComponent>().Translation = { 0, 0, 5.0f };
        }
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
    template<>
    void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity entity, Rigidbody2DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
    {
    }
    template<>
    void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component)
    {
    }
}