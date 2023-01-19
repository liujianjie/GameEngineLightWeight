#pragma once

#include <glm/glm.hpp>
#include "SceneCamera.h"
#include "Hazel/Scene/ScriptableEntity.h"
namespace Hazel {
    struct TagComponent { // 不用继承Component
        std::string Tag;
        TagComponent() = default;
        TagComponent(const TagComponent&) = default; // 复制构造函数
        TagComponent(const std::string& tag)          // 转换构造函数
            : Tag(tag) {}
    };
    struct TransformComponent { // 不用继承Component
        glm::mat4 Transform{ 1.0f };
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default; // 复制构造函数
        TransformComponent(const glm::mat4& transform)          // 转换构造函数
            : Transform(transform) {}
        operator const glm::mat4& () { return Transform; }      // 类型转换构造函数
        operator const glm::mat4& () const { return Transform; }
    };

    struct SpriteRendererComponent {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}
    };
    struct CameraComponent {
        SceneCamera camera;
        bool primary = true;
        bool fixedAspectRatio = false; // 这个是为了在游戏中不管视口怎么变都不会变得摄像机

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };
    struct NativeScriptComponent {
        ScriptableEntity* Instance = nullptr;
        // 关键地方//////////////////////////
        std::function<void()> InstantiateFunction;
        std::function<void()> DestroyInstanceFunction;

        std::function<void(ScriptableEntity*)> OnCreateFunction;
        std::function<void(ScriptableEntity*)> OnDestroyFunction;
        std::function<void(ScriptableEntity*, Timestep)> OnUpdateFunction;

        template<typename T>
        void Bind() {
            // 这里绑定的函数功能是：根据T动态实例化Instanse
            InstantiateFunction = [&]() {Instance = new T(); };// 引用值捕获Instance
            DestroyInstanceFunction = [&]() {delete (T*)Instance; Instance = nullptr; };// 为什么一定要转换为T，因为是在继承的情况下，起提示作用

            // 这里是绑定T的函数
            OnCreateFunction = [](ScriptableEntity* instance) {((T*)instance)->OnCreate(); };
            OnDestroyFunction = [](ScriptableEntity* instance) {((T*)instance)->OnDestroy(); };
            OnUpdateFunction = [](ScriptableEntity* instance, Timestep ts) {((T*)instance)->OnUpdate(ts); };
            /*
            create 同等写法
                std::function<void()> OnCreateFunction;
                OnCreateFunction = [&]() {((T*)Instance)->OnCreate(); }; // 这里隐式捕获的是this局部变量，Instance是全局的不能捕获，Instance前的this指针省略了
                nsc.OnCreateFunction();
            */
        }
    };
}