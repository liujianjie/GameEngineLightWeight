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
	typedef struct _MonoImage MonoImage;
}
namespace Hazel {
	// ��Ҫ�Ǽ��س�ʼ��mono��dll���Լ����ⲿ�ṩ����
	class ScriptEngine
	{
	public:
		static void Init();		// ��ʼ��
		static void Shutdown();	// �ر�

		static void LoadAssembly(const std::filesystem::path& filepath);		// 119.2.����dll����

		// 120.C#��ʵ�彻��
		static void OnRuntimeStart(Scene* scene);// �������п�ʼʱ
		static void OnRuntimeStop();				 // �������н���ʱ

		static bool EntityClassExists(const std::string& fullClassName);// ����Ƿ���ڴ˿ռ�+������ʵ��
		static void OnCreateEntity(Entity entity);						// �������п�ʼʱִ��
		static void OnUpdateEntity(Entity entity, Timestep ts);			// ��������ʱ

		static Scene* GetSceneContext();	

		// 121.���
		static MonoImage* GetCoreAssemblyImage();
	private:
		static void InitMono();												// 119.1.��ʼ��mono
		static void ShutdownMono();	// �ر�mono

		static MonoObject* InstantiateClass(MonoClass* monoClass);			// 119.4.����һ����MonoClass�๹�ɵ�mono�����ҳ�ʼ��
		friend class ScriptClass;

		// 120.C#��ʵ�彻��
		static void LoadAssemblyClasses(MonoAssembly* assembly);				// ����C#�и�����entity���࣬
	};
	// 120.C#��ʵ�彻��
	// �൱��new Class();�õ��Ķ����ٷ�װһ�㣬���ԣ����ĸ�Class��ʼ�����洢Class�ĺ�������Ϊ������Class�ĺ���
	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOncreate();			// ����C#���OnCreate����
		void InvokeOnUpdate(float ts);	// ����C#���OnUpdate����
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};
	// �൱�ڴ���Class����װ��Class�����ԣ����ĸ������ռ���������ش����ģ��洢������Class����Ϊ��ʵ����Class�ࡢ��ȡ��ĺ�����������ĺ���
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);				// 119.3. ����һ��MonoClass��

		MonoObject* Instantiate();																	// 119.4.����һ����MonoClass�๹�ɵ�mono�����ҳ�ʼ��
		MonoMethod* GetMethod(const std::string& name, int parameterCount);							// 119.5.1 ��ȡ��ĺ���
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 119.5.2 ������ĺ���
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
}