#include "hzpch.h"
#include "Scene.h"

#include "Components.h"
#include "Hazel/Renderer/Renderer2D.h"
#include <glm/glm.hpp>

//#include "SceneCamera.h"

#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"

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
    Entity Scene::CreateEntity(std::string name)
    {
        // 添加默认组件
        Entity entity = { m_Registry.create(),this };
        entity.AddComponent<TransformComponent>();
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
                // 3.1定义包围盒
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
            // Cherno说迭代速度，多久进行一次计算模拟。好奇这个6，是时间单位吗，毫秒？
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
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group) {
                // get返回的tuple里本是引用
                auto [tfc, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                //Renderer2D::DrawQuad(tfc.GetTransform(), sprite.Color);
                Renderer2D::DrawSprite(tfc.GetTransform(), sprite, (int)entity);
            }
            //if (group.size() <= 0) {
            //}
            Renderer2D::EndScene();
        }
    }
    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
        Renderer2D::BeginScene(camera);
        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group) {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            //Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color, (int)entity);
            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
        }
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
}