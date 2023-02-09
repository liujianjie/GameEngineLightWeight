#pragma once
#include <filesystem>
#include <string>
#include "Hazel/Scene/Scene.h"
#include "Hazel/Scene/Entity.h"

// 如果不引入头文件，必须外部声明，但这些都是在c文件定义的结构，所以需要extern"C"
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
		static void Init();		// 初始化
		static void Shutdown();	// 关闭

		static void LoadAssembly(const std::filesystem::path& filepath);		// 119.2.加载dll程序集

		// 120.C#与实体交互
		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullClassName);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep ts);

		static Scene* GetSceneContext();
	private:
		static void InitMono();												// 119.1.初始化mono
		static void ShutdownMono();	// 关闭mono

		static MonoObject* InstantiateClass(MonoClass* monoClass);			// 119.4.创建一个由MonoClass类构成的mono对象并且初始化
		friend class ScriptClass;

		// 120.C#与实体交互
		static void LoadAssemblyClasses(MonoAssembly* assembly);				// 加载C#中父类是entity的类，
	};
	// 120.C#与实体交互
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
		ScriptClass(const std::string& classNamespace, const std::string& className);				// 119.3. 创建一个MonoClass类

		MonoObject* Instantiate();// 4.创建一个由MonoClass类构成的mono对象并且初始化
		MonoMethod* GetMethod(const std::string& name, int parameterCount);							// 119.5.1 获取类的函数
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 119.5.2 调用类的函数
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
}