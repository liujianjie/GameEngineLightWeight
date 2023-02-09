#pragma once
#include <filesystem>
#include <string>

// ���������ͷ�ļ��������ⲿ����������Щ������c�ļ�����Ľṹ��������Ҫextern"C"
extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
}
namespace Hazel {
	class ScriptEngine
	{
	public:
		static void Init();		// ��ʼ��
		static void Shutdown();	// �ر�

		static void LoadAssembly(const std::filesystem::path& filepath);	// 2.����dll����
	private:
		static void InitMono();		// 1.��ʼ��mono
		static void ShutdownMono();	// �ر�mono

		static MonoObject* InstantiateClass(MonoClass* monoClass);	//
		friend class ScriptClass;
	};
	class ScriptClass {
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className);// 3. ����һ��MonoClass��

		MonoObject* Instantiate();// 4.����һ����MonoClass�๹�ɵ�mono�����ҳ�ʼ��
		MonoMethod* GetMethod(const std::string& name, int parameterCount);// 5.1 ��ȡ��ĺ���
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);// 5.2 ������ĺ���
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;
		MonoClass* m_MonoClass = nullptr;
	};
}