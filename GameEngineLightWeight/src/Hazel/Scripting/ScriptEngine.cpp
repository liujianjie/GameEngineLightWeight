#include "hzpch.h"
#include "ScriptEngine.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include <glm/gtc/matrix_transform.hpp>
namespace Hazel {

	struct ScriptEngineData {
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};
	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		InitMono();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	char* ReadBytes(const std::string& filepath, uint32_t* outSize) {
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		if (!stream) {
			// 打开文件失败
			return nullptr;
		}
		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();
		if (size == 0) {
			// 文件是空
			return nullptr;
		}
		char* buffer = new char[size];
		stream.read((char*)buffer, size); // 读入char字符数组中
		stream.close();

		*outSize = size; // 指针返回大小
		return buffer;	// 返回字符数组的首位置
	}
	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath) {
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);

		// 除了加载程序集之外，我们不能将此图像image用于任何其他用途，因为此图像没有对程序集的引用
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK) {
			const char* erroMessage = mono_image_strerror(status);
			// 可以打印错误信息
			return nullptr;
		}
		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		// 释放内存
		delete[] fileData;
		return assembly;
	}
	void PrintAssemblyTypes(MonoAssembly* assembly) {
		// 打印加载的c#程序的信息
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			HZ_CORE_TRACE("{}.{}", nameSpace, name);// 命名空间和类名
		}
	}
	static void CppFunc() {
		std::cout << "来自C++内部函数" << std::endl;
	}
	static void NativeLog(MonoString* string, int parameter) {
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);// 复制数据给str
		mono_free(cStr);
		std::cout << str << "," << parameter << std::endl;
	}
	static void NativeLogVec3(glm::vec3* vec, glm::vec3* out) {
		//HZ_CORE_WARN("Value: {0}", *vec); // 这会错的，并不支持输出向量
		std::cout << vec->x << "," << vec->y <<","<<vec->z << std::endl;
		*out = glm::cross(*vec, glm::vec3(vec->x, vec->y, -vec->z)); // 通过out返回指针
	}
	static glm::vec3* TestNativeLogVec3(glm::vec3* vec) {
		std::cout <<"TestNativeLogVec3" << std::endl;
		glm::vec3 result = glm::cross(*vec, glm::vec3(vec->x, vec->y, -vec->z)); // 通过out返回指针
		return new glm::vec3(result);
	}
	void ScriptEngine::InitMono()
	{
		// 设置程序集装配路径(复制的4.5版本的路径)
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomian = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomian);

		// 存储root domain指针
		s_Data->RootDomain = rootDomian;

		// 创建一个app domain
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// 添加内部调用
		mono_add_internal_call("Hazel.Main::CppFunction", CppFunc);
		mono_add_internal_call("Hazel.Main::NativeLog", NativeLog); 
		mono_add_internal_call("Hazel.Main::NativeLogVec3", NativeLogVec3);
		mono_add_internal_call("Hazel.Main::TestNativeLogVec3", TestNativeLogVec3);
		
		// 加载c#项目导出的dll
		s_Data->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/GameEngine-ScriptCore.dll");
		PrintAssemblyTypes(s_Data->CoreAssembly);// 打印dll的基本信息

		MonoImage* assemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		MonoClass* monoClass = mono_class_from_name(assemblyImage, "Hazel", "Main");

		// 1.创建一个Main类构成的mono对象并且初始化
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);

		// 2. 调用main类的函数-无参
		MonoMethod* printMessageFunc = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
		mono_runtime_invoke(printMessageFunc, instance, nullptr, nullptr);

		// 3.调用main类的函数-带参
		MonoMethod* printIntFunc = mono_class_get_method_from_name(monoClass, "PrintInt", 1);

		int value = 5;
		void* param = &value;

		mono_runtime_invoke(printIntFunc, instance, &param, nullptr);

		MonoMethod* printIntsFunc = mono_class_get_method_from_name(monoClass, "PrintInts", 2);

		int value2 = 505;
		void* params[2] = {
			&value,
			&value2
		};
		mono_runtime_invoke(printIntsFunc, instance, params, nullptr);

		// 带string的函数
		MonoString* monoString = mono_string_new(s_Data->AppDomain, "Hello World from C++!");
		MonoMethod* printCustomMessageFunc = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
		void* stringParam = monoString;
		mono_runtime_invoke(printCustomMessageFunc, instance, &stringParam, nullptr);

		 HZ_CORE_ASSERT(false);
	}

	void ScriptEngine::ShutdownMono()
	{
		// 对mono的卸载有点迷糊，以后再解决
		// mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		// mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}
}
