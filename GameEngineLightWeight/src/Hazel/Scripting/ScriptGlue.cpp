#include "hzpch.h"
#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"

#include "glm/glm.hpp"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include "box2d/b2_body.h"
namespace Hazel {

	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

	// mono_add_internal_call("Hazel.Main::CppFunction", CppFunc);
	#define HZ_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Hazel.InternalCalls::" #Name, Name)//InternalCalls����

	static void NativeLog(MonoString* string, int parameter) {
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);// �������ݸ�str
		mono_free(cStr);
		std::cout << str << "," << parameter << std::endl;
	}
	static void NativeLog_Vector(glm::vec3* vec, glm::vec3* out) {
		//HZ_CORE_WARN("Value: {0}", *vec); // ����ģ�����֧���������
		std::cout << vec->x << "," << vec->y << "," << vec->z << std::endl;
		//*out = glm::cross(*vec, glm::vec3(vec->x, vec->y, -vec->z)); // ͨ��out����ָ��
		*out = glm::normalize(*vec);
	}
	static float NativeLog_VectorDot(glm::vec3* vec) {
		std::cout << vec->x << "," << vec->y << "," << vec->z << std::endl;
		return glm::dot(*vec, *vec);
	}
	// 20230209���� 120��
	static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation) {
		//std::cout << "TransformComponent_GetTranslation" << std::endl;
		Scene* scene = ScriptEngine::GetSceneContext();// ��ȡ����
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID); // ����C#�����UUID�õ�Entity
		HZ_CORE_ASSERT(entity);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;// ����Entity��λ��
	}
	static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation) {
		//std::cout << "TransformComponent_SetTranslation" <<  std::endl;
		Scene* scene = ScriptEngine::GetSceneContext();	// ��ȡ����
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);// ����C#�����UUID�õ�Entity
		HZ_CORE_ASSERT(entity);

		entity.GetComponent<TransformComponent>().Translation = *translation;// ����Entity��λ��
	}
	static bool Input_IsKeyDown(KeyCode keycode) {
		return Input::IsKeyPressed(keycode);
	}
	// 121�ڣ����
	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType) {
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);
		// C#��typeof(T)��Sandbox.Player��ʵ�����󣬴�������Ӧ����ʵ������ĵ�ַ
		MonoType* managedType = mono_reflection_type_get_type(componentType);
		HZ_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(managedType)(entity);// �ҵ�������function
	}
	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake) {
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}
	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake) {
		Scene* scene = ScriptEngine::GetSceneContext();
		HZ_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		HZ_CORE_ASSERT(entity);

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}
	template<typename... Component>
	static void RegisterComponent() {
		([]()
			{
				std::string_view typeName = typeid(Component).name();
				size_t pos = typeName.find_last_of(':');
				std::string_view structName = typeName.substr(pos + 1);
				std::string managedTypname = fmt::format("Hazel.{}", structName);// ������ַ���ֻ����string����

				MonoType* managedType = mono_reflection_type_from_name(managedTypname.data(), ScriptEngine::GetCoreAssemblyImage()); // managedTypname.data() = managedType.ctr();
				if (!managedType) {
					HZ_CORE_ERROR("Could not find component type{}", managedTypname);
					return;
				}
				HZ_CORE_TRACE(managedTypname);
				s_EntityHasComponentFuncs[managedType] = [](Entity entity) {return entity.HasComponent<Component>(); };// Component��չ����ģ�������
			}(), ...);
	}
	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}
	void ScriptGlue::RegisterFunctions()
	{
		HZ_ADD_INTERNAL_CALL(NativeLog);// ���������ӦC#��C++ͬ������
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector);
		HZ_ADD_INTERNAL_CALL(NativeLog_VectorDot);

		HZ_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HZ_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		HZ_ADD_INTERNAL_CALL(Input_IsKeyDown);

		HZ_ADD_INTERNAL_CALL(Entity_HasComponent);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		HZ_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
	}
}