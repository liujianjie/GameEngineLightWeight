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

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		ScriptClass EntityClass;// �洢Entity����

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;// ����C#�ű�map
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;	// ��Ҫ���е�C#�ű�map

		// Runtime
		Scene* SceneContext = nullptr;	// ���������ģ�����C#����C++�ڲ�����ʱ����UUID��ȡ���������ʵ��
	};
	static ScriptEngineData* s_Data = nullptr;

	//////////////////////////////////////////////////////////////
	// ScriptEngine////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();
		// ��ʼ��mono
		InitMono();
		// ����c#����
		LoadAssembly("Resources/Scripts/GameEngine-ScriptCore.dll");				// ���Ŀ�
		LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");// ��Ϸ�ű���

		// ���ظ�����entity�Ľű���
		LoadAssemblyClasses();

		// ��������Entity����-Ϊ���ڵ���OnCreate����֮ǰ��UUID����C#Entity�Ĺ��캯��
		s_Data->EntityClass = ScriptClass("Hazel", "Entity", true);

		// ����ڲ�����
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
		// ���ó���װ��·��(���Ƶ�4.5�汾��·��)
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomian = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomian);

		// �洢root domainָ��
		s_Data->RootDomain = rootDomian;
	}
	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// ����һ��app domain
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// ����c#��Ŀ������dll
		s_Data->CoreAssembly = Utils::LoadCSharpAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		//Utils::PrintAssemblyTypes(s_Data->CoreAssembly);// ��ӡdll�Ļ�����Ϣ
	}
	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// ����c#��Ŀ������dll
		s_Data->AppAssembly = Utils::LoadCSharpAssembly(filepath);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		//Utils::PrintAssemblyTypes(s_Data->CoreAssembly);// ��ӡdll�Ļ�����Ϣ
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
		// ����һ��Main�๹�ɵ�mono�����ҳ�ʼ��
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);// ���캯�����������
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
		const auto& sc = entity.GetComponent<ScriptComponent>();		// �õ����ʵ������
		if (ScriptEngine::EntityClassExists(sc.ClassName)) {			// ����Ľű������Ƿ���ȷ
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);// ʵ���������
			s_Data->EntityInstances[entity.GetUUID()] = instance;	// ���нű�map�洢��ЩScriptInstance(�����)
			instance->InvokeOncreate();								// ����C#��OnCreate����
		}
	}
	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();							// �õ����ʵ���UUID
		HZ_CORE_ASSERT(s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end());

		// ����UUID��ȡ��ScriptInstance��ָ��
		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);							// ����C#��OnUpdate����
	}
	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		// 1.����Entity����
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
			// 2.����Dll������C#��
			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, name);
			if (monoClass == entityClass) {// entity���಻����
				continue;
			}
			// 3.�жϵ�ǰ���Ƿ�ΪEntity������
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false); // ���c#���Ƿ�Ϊentity������
			if (isEntity) {
				// 3.1�Ǿʹ���ű�map��
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
		// ��ȡSandbox Player�๹�ɵ�MonoObject�����൱��new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();	

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);// ��ȡC#Entity��Ĺ��캯��
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);// ��ȡ���洢Sandbox.Player��ĺ���
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		// ����C#Entity��Ĺ��캯��
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);// ��һ�������������Entity����(Player)���ɵ�mono����
		}
	}
	void ScriptInstance::InvokeOncreate()
	{
		if (m_OnCreateMethod) {
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);// ����Sandbox.Player���OnUpdate����
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
		return mono_runtime_invoke(method, instance, params, nullptr);	// **���ͣ�&params(ʵ��) = params��ʵ�Σ�
	}
}
