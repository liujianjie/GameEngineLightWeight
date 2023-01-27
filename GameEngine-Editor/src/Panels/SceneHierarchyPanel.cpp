#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Hazel/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
namespace Hazel {
	// transform�����ui
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
		// ����UI���Ӵְ�ť����
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0]; // ��ȡ����

		// ImGUi push���� Ҫpop���٣���Ȼ�ᱨ��
		ImGui::PushID(label.c_str()); // ÿһ����label��ID��3��ID��ͬ��������

		// ����һ������
		ImGui::Columns(2);
		// ��һ��
		ImGui::SetColumnWidth(0, columnWidth);// ���õ�1�п�100
		ImGui::Text(label.c_str());
		ImGui::NextColumn();		

		// �ڶ���
		// ����3��item�Ŀ�
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;// �����и�
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };// ��ť��С

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		// ����UI���Ӵ�����
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize) ){
			values.x = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		// ��ΪDragFloat button�ỻ�У���������SameLine��������
		ImGui::SameLine();
		// ##X������һ��id����##x������UI������ʾ������
		// #X ����ʾ���ı�����ұ�
		// X  ���������BUttonͬ�� ͬid�������Ļ��ᱨ��
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize)) {
			values.y = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");// 0.1�ٶȣ�0 - 0 ��С���������
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize)) {
			values.z = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		// �ָ���һ��һ��
		ImGui::Columns(1);

		ImGui::PopID();
	}
	
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
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

		// �һ��հ����-�����˵���0��ID 1���Ҽ�
		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Create Empty Entity")) {
				m_Context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}
		ImGui::End();
		// �жϵ�ǰ�����ʵ���Ƿ����
		ImGui::Begin("Properties");
		if (m_SelectionContext) { // operator uint32_t() ����const����Ȼ�������operator bool(),���ǵ���uint32_t()
			DrawComponents(m_SelectionContext);
			
		}
		ImGui::End();
	}
	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		// Ҫ���� Logͷ�ļ�
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		// ���Ǳ�������Ϊѡ��״̬|����һ��
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		// ����UI1: ����Tree�Ŀ��Ϊ����
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// ��һ��������ΨһID 64�ģ�
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_SelectionContext = entity; // ��¼��ǰ�����ʵ��
		}
		// �Ҽ�ʵ��-�����˵�
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Delete Entity")) {
				entityDeleted = true;
			}
			ImGui::EndPopup();
		}
		if (opened) {
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)98476565, flags, tag.c_str());
			if (opened) {
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		// �Ӻ�ɾ��
		if (entityDeleted) {
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity) {
				m_SelectionContext = {};
			}
		}
	}
	// ����UI����ģ��+lambda�滻�����ui���ơ�����2�ǲ��������ƶϳ�����
	template<typename T,typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>()) {
			//ImGui::Separator();
			auto& component = entity.GetComponent<T>();
			// Ϊ�˶�λ+��ť�� ���ұ�
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator(); // ˮƽ����ߣ����Է�����Ҳ��
			// �Ȼ���������
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			// �ٻ��ư�ť
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f); // ������һ�����ͬһ�У������ڸ���������ұ�
			// �����ť-�����˵�
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) {
				ImGui::OpenPopup("ComponentSettings");
			}
			//ImGui::PopStyleVar();

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings")) {
				if (ImGui::MenuItem("Remove component")) {
					removeComponent = true;
				}
				ImGui::EndPopup();
			}
			if (open) {
				uiFunction(component);// ����lambda����
				// չ�����ڵ�
				ImGui::TreePop();
			}
			// �ӳ�ɾ��
			if (removeComponent) {
				entity.RemoveComponent<T>();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		// ʵ������
		if (entity.HasComponent<TagComponent>()) {
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			// һ���ַ��������������˴�С������̫��
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));

			strcpy(buffer, tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}
		// ����UI������������ť�Ƶ�������
		// �����������ʾ��������ť����������˵�
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);// ˵ʲô������������ұ�
		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponent");// AddComponentֻ��id
		}
		if (ImGui::BeginPopup("AddComponent")) {
			if (ImGui::MenuItem("Camera")) {
				m_SelectionContext.AddComponent<CameraComponent>();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::MenuItem("Sprite Renderer")) {
				m_SelectionContext.AddComponent<SpriteRendererComponent>();
			}
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		// ʵ��transform���
		// ����UI��ģ���໭�����ui
		DrawComponent<TransformComponent>("Transform", entity, [](auto& tfc)
		{
			DrawVec3Control("Translation", tfc.Translation);
			glm::vec3 rotation = glm::degrees(tfc.Rotation);
			DrawVec3Control("Rotation", rotation); // ������ʾ�Ƕ�
			tfc.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", tfc.Scale, 1.0f);
		});
		// ��������
		// ����UI��ģ���໭�����ui
		DrawComponent<CameraComponent>("Camera", entity, [](auto& cameraComponent)
		{
			auto& camera = cameraComponent.camera;
			ImGui::Checkbox("Primary", &cameraComponent.primary);
			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString)) {
				for (int i = 0; i < 2; i++) {
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected)) {
						currentProjectionTypeString = projectionTypeStrings[i];
						// ��������������ͣ����¼���ͶӰ����
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV()); // ת��Ϊ�Ƕ�
				if (ImGui::DragFloat("Vertical FOV", &verticalFov)) {
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov)); // ���ûػ���
				}

				float orthoNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near", &orthoNear)) {
					camera.SetPerspectiveNearClip(orthoNear);
				}

				float orthoFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far", &orthoFar)) {
					camera.SetPerspectiveFarClip(orthoFar);
				}
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic) {
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize)) {
					camera.SetOrthographicSize(orthoSize);
				}

				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near", &orthoNear)) {
					camera.SetOrthographicNearClip(orthoNear);
				}
				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far", &orthoFar)) {
					camera.SetOrthographicFarClip(orthoFar);
				}
				ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.fixedAspectRatio);
			}
		});
		// ʵ��SpriteRendererComponent���		
		// ����UI��ģ���໭�����ui
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
		});
	}
}