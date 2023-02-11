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
		MonoAssembly* LoadCSharpAssembly(const std::filesystem::path& assemblyPath) {
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
			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
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
	}
	struct ScriptEngineData {
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		ScriptClass EntityClass;// 存储Entity父类

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;// 所有C#脚本map
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;	// 需要运行的C#脚本map

		// Runtime
		Scene* SceneContext = nullptr;	// 场景上下文，用在C#调用C++内部函数时根据UUID获取这个场景的实体
	};
	static ScriptEngineData* s_Data = nullptr;

	//////////////////////////////////////////////////////////////
	// ScriptEngine////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		// 初始化mono
		InitMono();
		// 加载c#程序集
		LoadAssembly("Resources/Scripts/GameEngine-ScriptCore.dll");				// 核心库
		LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");// 游戏脚本库

		// 加载父类是entity的脚本类
		LoadAssemblyClasses();

		// 创建加载Entity父类-为了在调用OnCreate函数之前把UUID传给C#Entity的构造函数
		s_Data->EntityClass = ScriptClass("Hazel", "Entity", true);

		// 添加内部调用
		ScriptGlue::RegisterFunctions();
		ScriptGlue::RegisterComponents();

		//HZ_CORE_ASSERT(false);
	}
	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}
	void ScriptEngine::InitMono()
	{
		// 设置程序集装配路径(复制的4.5版本的路径)
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomian = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomian);

		// 存储root domain指针
		s_Data->RootDomain = rootDomian;
	}
	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// 创建一个app domain
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// 加载c#项目导出的dll
		s_Data->CoreAssembly = Utils::LoadCSharpAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		//Utils::PrintAssemblyTypes(s_Data->CoreAssembly);// 打印dll的基本信息
	}
	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// 加载c#项目导出的dll
		s_Data->AppAssembly = Utils::LoadCSharpAssembly(filepath);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		//Utils::PrintAssemblyTypes(s_Data->CoreAssembly);// 打印dll的基本信息
	}
	void ScriptEngine::ShutdownMono()
	{
		// 对mono的卸载有点迷糊，以后再解决
		// mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		// mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}
	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		// 创建一个Main类构成的mono对象并且初始化
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);// 构造函数在这里调用
		return instance;
	}
	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}
	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;
		s_Data->EntityInstances.clear();
	}
	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}
	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();		// 得到这个实体的组件
		if (ScriptEngine::EntityClassExists(sc.ClassName)) {			// 组件的脚本名称是否正确
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);// 实例化类对象
			s_Data->EntityInstances[entity.GetUUID()] = instance;	// 运行脚本map存储这些ScriptInstance(类对象)
			instance->InvokeOncreate();								// 调用C#的OnCreate函数
		}
	}
	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();							// 得到这个实体的UUID
		HZ_CORE_ASSERT(s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end());

		// 根据UUID获取到ScriptInstance的指针
		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);							// 调用C#的OnUpdate函数
	}
	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		// 1.加载Entity父类
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Hazel", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0) {
				fullName = fmt::format("{}.{}", nameSpace, name);
			}
			else {
				fullName = name;
			}
			// 2.加载Dll中所有C#类
			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, name);
			if (monoClass == entityClass) {// entity父类不保存
				continue;
			}
			// 3.判断当前类是否为Entity的子类
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false); // 这个c#类是否为entity的子类
			if (isEntity) {
				// 3.1是就存入脚本map中
				s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
	}
	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}
	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}
	//////////////////////////////////////////////////////////////
	// ScriptInstance/////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		:m_ScriptClass(scriptClass)
	{
		// 获取Sandbox Player类构成的MonoObject对象，相当于new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();	

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);// 获取C#Entity类的构造函数
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);// 获取并存储Sandbox.Player类的函数
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		// 调用C#Entity类的构造函数
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);// 第一个参数传入的是Entity子类(Player)构成的mono对象
		}
	}
	void ScriptInstance::InvokeOncreate()
	{
		if (m_OnCreateMethod) {
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);// 调用Sandbox.Player类的OnUpdate函数
		}
	}
	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod) {
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}
	//////////////////////////////////////////////////////////////
	// ScriptClass////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
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
		return mono_runtime_invoke(method, instance, params, nullptr);	// **类型，&params(实参) = params（实参）
	}
}
