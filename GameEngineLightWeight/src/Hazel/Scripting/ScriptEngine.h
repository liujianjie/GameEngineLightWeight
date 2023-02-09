#pragma once
#include <filesystem>
#include <string>

// 如果不引入头文件，必须外部声明，但这些都是在c文件定义的结构，所以需要extern"C"
extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}
namespace Hazel {
	class ScriptEngine
	{
	public:
		static void Init();		// 初始化
		static void Shutdown();	// 关闭

		static void LoadAssembly(const std::filesystem::path& filepath);	// 2.加载dll程序集
	private:
		static void InitMono();		// 1.初始化mono
		static void ShutdownMono();	// 关闭mono

		static MonoObject* InstantiateClass(MonoClass* monoClass);	//
		friend class ScriptClass;
	};
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);// 3. 创建一个MonoClass类

		MonoObject* Instantiate();// 4.创建一个由MonoClass类构成的mono对象并且初始化
		MonoMethod* GetMethod(const std::string& name, int parameterCount);// 5.1 获取类的函数
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 5.2 调用类的函数
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
}