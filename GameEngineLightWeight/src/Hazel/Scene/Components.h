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
        // 用函数指针
        ScriptableEntity* (*InstantiateScript)();// 这函数返回ScriptableEntity指针，函数无参数，InstantiateScript的*代表为指针
        void(*DestroyScript)(NativeScriptComponent*);
        template<typename T>
        void Bind() {
            // 这里绑定的函数功能是：根据T动态实例化Instanse
            InstantiateScript = []() {return static_cast<ScriptableEntity*>(new T()); };// 引用值捕获Instance
            DestroyScript = [](NativeScriptComponent* nsc) {delete nsc->Instance; nsc->Instance = nullptr; };// 感觉参数放不放NativeScriptComponent无所谓，反正有this
        }
    };
}