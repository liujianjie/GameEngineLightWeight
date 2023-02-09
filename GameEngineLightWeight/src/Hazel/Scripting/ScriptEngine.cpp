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

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// Runtime
		Scene* SceneContext = nullptr;
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
		// 3.1���ظ�����entity�Ľű���
		LoadAssemblyClasses(s_Data->CoreAssembly);

		// ����ڲ�����
		ScriptGlue::RegisterFunctions();

		// 3.2��������Entity��
		s_Data->EntityClass = ScriptClass("Hazel", "Entity");


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
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptEngine::EntityClassExists(sc.ClassName)) {
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
			s_Data->EntityInstances[entity.GetUUID()] = instance;
			instance->InvokeOncreate();
		}
	}
	void Hazel::ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		HZ_CORE_ASSERT(s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end());

		// ����UUID��ȡ��ScriptInstance��ָ��
		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts);
	}
	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_Data->EntityClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(image, "Hazel", "Entity");// ������

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0) {
				fullName = fmt::format("{}.{}", nameSpace, name);
			}
			else {
				fullName = name;
			}
			// **������õ�C#��**
			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);
			if (monoClass == entityClass) {// entity���಻����
				continue;
			}
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false); // ���c#���Ƿ�Ϊentity������
			if (isEntity) {
				s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
	}
	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}
	//////////////////////////////////////////////////////////////
	// ScriptInstance////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		:m_ScriptClass(scriptClass)
	{
		// ��ȡSandbox Player�๹�ɵ�MonoObject�����൱��new Sandbox.Player()
		m_Instance = scriptClass->Instantiate();	

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);// ��ȡC#Entity��Ĺ��캯��
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);// ��ȡSandbox.Player��ĺ���
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		// ����C#Entity��Ĺ��캯��
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);// �������Entity����(Player)���ɵ�mono����
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
