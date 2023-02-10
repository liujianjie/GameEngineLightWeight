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
	typedef struct _MonoImage MonoImage;
}
namespace Hazel {
	// 主要是加载初始化mono和dll，以及向外部提供调用
	class ScriptEngine
	{
	public:
		static void Init();		// 初始化
		static void Shutdown();	// 关闭

		static void LoadAssembly(const std::filesystem::path& filepath);		// 119.2.加载dll程序集

		// 120.C#与实体交互
		static void OnRuntimeStart(Scene* scene);// 场景运行开始时
		static void OnRuntimeStop();				 // 场景运行结束时

		static bool EntityClassExists(const std::string& fullClassName);// 检测是否存在此空间+类名的实体
		static void OnCreateEntity(Entity entity);						// 场景运行开始时执行
		static void OnUpdateEntity(Entity entity, Timestep ts);			// 场景运行时

		static Scene* GetSceneContext();	

		// 121.组件
		static MonoImage* GetCoreAssemblyImage();
	private:
		static void InitMono();												// 119.1.初始化mono
		static void ShutdownMono();	// 关闭mono

		static MonoObject* InstantiateClass(MonoClass* monoClass);			// 119.4.创建一个由MonoClass类构成的mono对象并且初始化
		friend class ScriptClass;

		// 120.C#与实体交互
		static void LoadAssemblyClasses(MonoAssembly* assembly);				// 加载C#中父类是entity的类，
	};
	// 120.C#与实体交互
	// 相当于new Class();得到的对象再封装一层，属性：由哪个Class初始化、存储Class的函数，行为：调用Class的函数
	class ScriptInstance {
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOncreate();			// 调用C#类的OnCreate函数
		void InvokeOnUpdate(float ts);	// 调用C#类的OnUpdate函数
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
	};
	// 相当于创建Class，封装此Class，属性：由哪个命名空间和类名加载创建的，存储创建的Class，行为：实例化Class类、获取类的函数、调用类的函数
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);				// 119.3. 创建一个MonoClass类

		MonoObject* Instantiate();																	// 119.4.创建一个由MonoClass类构成的mono对象并且初始化
		MonoMethod* GetMethod(const std::string& name, int parameterCount);							// 119.5.1 获取类的函数
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 119.5.2 调用类的函数
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
}