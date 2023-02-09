#pragma once
#include <filesystem>
#include <string>
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

// ���������ͷ�ļ��������ⲿ����������Щ������c�ļ�����Ľṹ��������Ҫextern"C"
extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
}
namespace Hazel {
	class ScriptEngine
	{
	public:
		static void Init();		// ��ʼ��
		static void Shutdown();	// �ر�

		static void LoadAssembly(const std::filesystem::path& filepath);		// 119.2.����dll����

		// 120.C#��ʵ�彻��
		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullClassName);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
	private:
		static void InitMono();												// 119.1.��ʼ��mono
		static void ShutdownMono();	// �ر�mono

		static MonoObject* InstantiateClass(MonoClass* monoClass);			// 119.4.����һ����MonoClass�๹�ɵ�mono�����ҳ�ʼ��
		friend class ScriptClass;

		// 120.C#��ʵ�彻��
		static void LoadAssemblyClasses(MonoAssembly* assembly);				// ����C#�и�����entity���࣬
	};
	// 120.C#��ʵ�彻��
	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOncreate();
		void InvokeOnUpdate(float ts);
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);				// 119.3. ����һ��MonoClass��

		MonoObject* Instantiate();// 4.����һ����MonoClass�๹�ɵ�mono�����ҳ�ʼ��
		MonoMethod* GetMethod(const std::string& name, int parameterCount);							// 119.5.1 ��ȡ��ĺ���
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 119.5.2 ������ĺ���
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
}