#pragma once

#include <glm/glm.hpp>
#include "SceneCamera.h"
#include "Hazel/Scene/ScriptableEntity.h"
namespace Hazel {
    struct TagComponent { // ���ü̳�Component
        std::string Tag;
        TagComponent() = default;
        TagComponent(const TagComponent&) = default; // ���ƹ��캯��
        TagComponent(const std::string& tag)          // ת�����캯��
            : Tag(tag) {}
    };
    struct TransformComponent { // ���ü̳�Component
        glm::mat4 Transform{ 1.0f };
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default; // ���ƹ��캯��
        TransformComponent(const glm::mat4& transform)          // ת�����캯��
            : Transform(transform) {}
        operator const glm::mat4& () { return Transform; }      // ����ת�����캯��
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
        bool fixedAspectRatio = false; // �����Ϊ������Ϸ�в����ӿ���ô�䶼�����������

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };
    struct NativeScriptComponent {
        ScriptableEntity* Instance = nullptr;
        // �ú���ָ��
        ScriptableEntity* (*InstantiateScript)();// �⺯������ScriptableEntityָ�룬�����޲�����InstantiateScript��*����Ϊָ��
        void(*DestroyScript)(NativeScriptComponent*);
        template<typename T>
        void Bind() {
            // ����󶨵ĺ��������ǣ�����T��̬ʵ����Instanse
            InstantiateScript = []() {return static_cast<ScriptableEntity*>(new T()); };// ����ֵ����Instance
            DestroyScript = [](NativeScriptComponent* nsc) {delete nsc->Instance; nsc->Instance = nullptr; };// �о������Ų���NativeScriptComponent����ν��������this
        }
    };
}