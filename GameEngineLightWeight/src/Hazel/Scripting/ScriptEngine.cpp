#include "hzpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include <glm/gtc/matrix_transform.hpp>
namespace Hazel {
	namespace Utils {
		char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize) {
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
			if (!stream) {
				// ���ļ�ʧ��
				return nullptr;
			}
			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();
			if (size == 0) {
				// �ļ��ǿ�
				return nullptr;
			}
			char* buffer = new char[size];
			stream.read((char*)buffer, size); // ����char�ַ�������
			stream.close();

			*outSize = size; // ָ�뷵�ش�С
			return buffer;	// �����ַ��������λ��
		}
		MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath) {
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			// ���˼��س���֮�⣬���ǲ��ܽ���ͼ��image�����κ�������;����Ϊ��ͼ��û�жԳ��򼯵�����
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK) {
				const char* erroMessage = mono_image_strerror(status);
				// ���Դ�ӡ������Ϣ
				return nullptr;
			}
			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			// �ͷ��ڴ�
			delete[] fileData;
			return assembly;
		}
		void PrintAssemblyTypes(MonoAssembly* assembly) {
			// ��ӡ���ص�c#�������Ϣ
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				HZ_CORE_TRACE("{}.{}", nameSpace, name);// �����ռ������
			}
		}
	}
	struct ScriptEngineData {
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;
		ScriptClass EntityClass;
	};
	static ScriptEngineData* s_Data = nullptr;

	//////////////////////////////////////////////////////////////
	// ScriptEngine////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		// 1 ��ʼ��mono
		InitMono();
		// 2.����c#����
		LoadAssembly("Resources/Scripts/GameEngine-ScriptCore.dll");

		// ����ڲ�����
		ScriptGlue::RegisterFunctions();

		// 3 ����һ��MonoClass��
		s_Data->EntityClass = ScriptClass("Hazel", "Entity");

		// 4.����һ��Main�๹�ɵ�mono�����ҳ�ʼ��
		MonoObject* instance = s_Data->EntityClass.Instantiate();

		// 5.1����main��ĺ���-�޲�
		MonoMethod* printMessageFunc = s_Data->EntityClass.GetMethod("PrintMessage", 0);
		s_Data->EntityClass.InvokeMethod(instance, printMessageFunc);

		// 5.2����main��ĺ���-����
		MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInt", 1);

		int value = 5;
		void* param = &value;

		mono_runtime_invoke(printIntFunc, instance, &param, nullptr);

		MonoMethod* printIntsFunc = s_Data->EntityClass.GetMethod("PrintInts", 2);

		int value2 = 505;
		void* params[2] = {
			&value,
			&value2
		};
		mono_runtime_invoke(printIntsFunc, instance, params, nullptr);

		// ��string�ĺ���
		MonoString* monoString = mono_string_new(s_Data->AppDomain, "Hello World from C++!");
		MonoMethod* printCustomMessageFunc = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
		void* stringParam = monoString;
		mono_runtime_invoke(printCustomMessageFunc, instance, &stringParam, nullptr);

		//HZ_CORE_ASSERT(false);
	}
	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}
	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// ����һ��app domain
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// ����c#��Ŀ������dll
		s_Data->CoreAssembly = Utils::LoadCSharpAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		Utils::PrintAssemblyTypes(s_Data->CoreAssembly);// ��ӡdll�Ļ�����Ϣ
	}
	
	void ScriptEngine::InitMono()
	{
		// ���ó���װ��·��(���Ƶ�4.5�汾��·��)
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomian = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomian);

		// �洢root domainָ��
		s_Data->RootDomain = rootDomian;

	}
	void ScriptEngine::ShutdownMono()
	{
		// ��mono��ж���е��Ժ����Ժ��ٽ��
		// mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		// mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}
	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		// 1.����һ��Main�๹�ɵ�mono�����ҳ�ʼ��
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);// ���캯�����������
		return instance;
	}
	//////////////////////////////////////////////////////////////
	// ScriptClass////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
	{
		m_MonoClass = mono_class_from_name(s_Data->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
	}
	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}
	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{		
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}
	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		return mono_runtime_invoke(method, instance, params, nullptr);
	}
}
