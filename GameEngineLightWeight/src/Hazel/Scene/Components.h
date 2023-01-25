#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
        // ���transform�Ǿ��󰡣�����ʵ���xyzλ�ã�Transform[3]����xyzλ�á�
        // ������Ϊ0����Ϊ0����������Ϊ0���Ͳ���ʾ��
        //glm::mat4 Transform{ 1.0f }; 
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f,0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default; // ���ƹ��캯��
        TransformComponent(const glm::vec3& translation)          // ת�����캯��
            : Translation(translation) {}
        glm::mat4 GetTransform()const {
            //glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1,0,0 })
            //    * glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
            //    * glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });
            // ����Ԫ����þ���
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
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