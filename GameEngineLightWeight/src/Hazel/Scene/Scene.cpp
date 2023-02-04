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

    // 自定义的物理类型转换为box2d的类型
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
    // 为复制场景的实体的组件的辅助方法
    template<typename Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap) {
        auto view = src.view<Component>();
        // 2.1遍历旧场景所有uuid组件的旧实体
        for (auto e : view) {
            UUID uuid = src.get<IDComponent>(e).ID;
            HZ_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
            // 2.2用** 旧实体的uuid - map - 对应新实体 * *
            entt::entity dstEnttID = enttMap.at(uuid);
            // 3.1获取旧实体的组件
            auto& component = src.get<Component>(e);
            // 3.2然后用API，** 复制旧实体的组件给新实体**
            dst.emplace_or_replace<Component>(dstEnttID, component);// 添加或替换，保险
        }
    }
    // 为复制实体的辅助方法
    template<typename Component>
    static void CopyComponentIfExists(Entity dst, Entity src) {
        if (src.HasComponent<Component>()) {
            dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
        }
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        // 1.1创建新场景
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
            // 1.2为新场景创建和旧场景同名和uuid的实体
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            // 1.3并用**map存入（旧实体的uuid对应新实体）的关系**
            enttMap[uuid] = (entt::entity)newEntity;// UUID类需要哈希
        }

        // 拷贝组件，除了IDcomponent与tagcomponent，因CreateEntityWithUUID创建了
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
        // 1.创建旧实体同名的新实体
        std::string name = entity.GetName();
        Entity newEntity = CreateEntity(name);
        // 2.复制组件
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
        // 创建新的uuid
        return CreateEntityWithUUID(UUID(), name);
    }
    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        // 添加默认组件
        Entity entity = { m_Registry.create(),this };
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<IDComponent>(uuid); // 使用存在的uuid，不创建新的
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }
    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }
    void Scene::OnRuntimeStart()
    {
        // 1.创建一个物体世界
        m_PhysicsWorld = new b2World({0.0f, -9.8f});// 重力加速度向下
        // 1.1为当前场景所有具有物理组件的实体创建b2Body
        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view) {
            Entity entity = { e, this };
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            // 2.1 主体定义用来指定动态类型和参数
            b2BodyDef bodyDef;
            bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;   // 绕着z轴旋转
            // 2.2 由b2BodyDef创建主体
            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation); // 是否固定旋转

            rb2d.RuntimeBody = body;

            if (entity.HasComponent<BoxCollider2DComponent>()) {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                // 3.1定义Box包围盒
                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);// 包围盒跟随物体的size而变化
                // 3.2定义fixture，fixture包含定义的包围盒
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = bc2d.Density;
                fixtureDef.friction = bc2d.Friction;
                fixtureDef.restitution = bc2d.Restitution;
                fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;
                // 3.3定义主体的fixture
                body->CreateFixture(&fixtureDef);
            }
            if (entity.HasComponent<CircleCollider2DComponent>()) {
                auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
                // 3.1定义圆形包围盒
                b2CircleShape circleShape;
                circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
                circleShape.m_radius = cc2d.Radius;
                // 3.2定义fixture，fixture包含定义的包围盒
                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circleShape;
                fixtureDef.density = cc2d.Density;
                fixtureDef.friction = cc2d.Friction;
                fixtureDef.restitution = cc2d.Restitution;
                fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;
                // 3.3定义主体的fixture
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
        {
            // 先script脚本影响Physics变化再当前帧渲染出来
            // 迭代速度：使用更少的迭代可以提高性能，但准确性会受到影响。使用更多迭代会降低性能但会提高模拟质量
            // 有点不董。。。。说啥：时间步长和迭代次数完全无关。迭代不是子步骤
            // Cherno说迭代速度，多久进行一次计算模拟。好奇这个6，是多少毫秒计算6次吗？
            const int32_t velocityIterations = 6;// 这些参数应该移到编辑器
            const int32_t positionIterations = 2;
            m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

            auto view = m_Registry.view<Rigidbody2DComponent>();
            for (auto e : view) {
                Entity entity = { e, this };
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                // 获取物理模拟计算后的主体
                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                // 将计算后的值赋予实体
                const auto& position = body->GetPosition();
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = body->GetAngle();// 获取z轴角度
            }
            // 脚本影响Pyhsics再下面渲染出来
        }
        if (mainCamera) {
            Renderer2D::BeginScene(*mainCamera, cameraTransform);
            // 绘画sprite
            {
                auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                for (auto entity : group) {
                    // get返回的tuple里本是引用
                    auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                    Renderer2D::DrawSprite(tfc.GetTransform(), sprite, (int)entity);
                }
            }
            // 绘画circles
            {
                auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                for (auto entity : view) {
                    // get返回的tuple里本是引用
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
        // 绘画sprite
        {
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group) {
                // get返回的tuple里本是引用
                auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawSprite(tfc.GetTransform(), sprite, (int)entity);
                //Renderer2D::DrawRect(tfc.GetTransform(), glm::vec4(1.0f), (int)entity);
            }
        }
        // 绘画circles
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view) {
                // get返回的tuple里本是引用
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
    // 模板类定义在cpp中
    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
        // 静态断言：false，代表在编译前就会执行， 但是编译器这里不会报错，说明这段代码不会编译吧。。
        // 而且打了断点，也不行，证明这段代码只是声明作用吧。
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
        // 如果是透视摄像机，并且z=0，就重新设置
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