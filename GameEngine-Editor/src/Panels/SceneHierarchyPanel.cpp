#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include "Hazel/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
namespace Hazel {
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		m_Context->m_Registry.each([&](auto entityID){
			Entity entity{entityID, m_Context.get()};
			DrawEntityNode(entity);
		});
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			m_SelectionContext = {};
		}
		ImGui::End();
		// 判断当前点击的实体是否存在
		ImGui::Begin("Properties");
		if (m_SelectionContext) { // operator uint32_t() 的是const，不然不会调用operator bool(),而是调用uint32_t()
			DrawComponents(m_SelectionContext);
		}
		ImGui::End();
	}
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		// 要引入 Log头文件
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		// 若是被点击标记为选中状态|有下一级
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		// 第一个参数是唯一ID 64的，
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_SelectionContext = entity; // 记录当前点击的实体
		}
		if (opened) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
			bool opened = ImGui::TreeNodeEx((void*)98476565, flags, tag.c_str());
			if (opened) {
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		// 实体名称
		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			// 一个字符缓冲区，限制了大小，以免太长
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));

			strcpy(buffer, tag.c_str());
			if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}
		// 实体transform组件
		if (entity.HasComponent<TransformComponent>()) {
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
				auto& transform = entity.GetComponent<TransformComponent>().Transform;
				ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);// 0.1f是拖动文本框的步幅
				// 展开树节点
				ImGui::TreePop();
			}
		}
	}
}