#include "hzpch.h"
#include "ScriptGlue.h"

#include "glm/glm.hpp"
#include "mono/metadata/object.h"
namespace Hazel {
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
	void ScriptGlue::RegisterFunctions()
	{
		HZ_ADD_INTERNAL_CALL(NativeLog);// ���������ӦC#��C++ͬ������
		HZ_ADD_INTERNAL_CALL(NativeLog_Vector);
		HZ_ADD_INTERNAL_CALL(NativeLog_VectorDot);
	}
}