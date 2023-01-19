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
        // �ؼ��ط�//////////////////////////
        std::function<void()> InstantiateFunction;
        std::function<void()> DestroyInstanceFunction;

        std::function<void(ScriptableEntity*)> OnCreateFunction;
        std::function<void(ScriptableEntity*)> OnDestroyFunction;
        std::function<void(ScriptableEntity*, Timestep)> OnUpdateFunction;

        template<typename T>
        void Bind() {
            // ����󶨵ĺ��������ǣ�����T��̬ʵ����Instanse
            InstantiateFunction = [&]() {Instance = new T(); };// ����ֵ����Instance
            DestroyInstanceFunction = [&]() {delete (T*)Instance; Instance = nullptr; };// Ϊʲôһ��Ҫת��ΪT����Ϊ���ڼ̳е�����£�����ʾ����

            // �����ǰ�T�ĺ���
            OnCreateFunction = [](ScriptableEntity* instance) {((T*)instance)->OnCreate(); };
            OnDestroyFunction = [](ScriptableEntity* instance) {((T*)instance)->OnDestroy(); };
            OnUpdateFunction = [](ScriptableEntity* instance, Timestep ts) {((T*)instance)->OnUpdate(ts); };
            /*
            create ͬ��д��
                std::function<void()> OnCreateFunction;
                OnCreateFunction = [&]() {((T*)Instance)->OnCreate(); }; // ������ʽ�������this�ֲ�������Instance��ȫ�ֵĲ��ܲ���Instanceǰ��thisָ��ʡ����
                nsc.OnCreateFunction();
            */
        }
    };
}