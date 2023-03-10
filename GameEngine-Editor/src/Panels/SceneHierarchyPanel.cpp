#include "hzpch.h"
#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Hazel/Scene/Components.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Hazel/Scripting/ScriptEngine.h"
namespace Hazel {
	extern const std::filesystem::path g_AssetPath;
	// transform组件的ui
	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f) {
		// 完善UI：加粗按钮字体
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0]; // 获取字体

		// ImGUi push多少 要pop多少，不然会报错
		ImGui::PushID(label.c_str()); // 每一行用label做ID，3行ID不同互不干扰

		// 设置一行两列
		ImGui::Columns(2);
		// 第一列
		ImGui::SetColumnWidth(0, columnWidth);// 设置第1列宽100
		ImGui::Text(label.c_str());
		ImGui::NextColumn();		

		// 第二列
		// 放入3个item的宽
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;// 设置行高
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };// 按钮大小

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		// 完善UI：加粗字体
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize) ){
			values.x = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		// 因为DragFloat button会换行，所以设置SameLine将不换行
		ImGui::SameLine();
		// ##X将分配一个id，且##x不会在UI界面显示出来。
		// #X 将显示在文本框的右边
		// X  将与上面的BUtton同名 同id，操作的话会报错
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
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");// 0.1速度，0 - 0 最小最大无限制
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

		// 恢复成一行一列
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

		// 右击空白面板-弹出菜单。0是ID 1是右键
		if (ImGui::BeginPopupContextWindow(0, 1, false)) {
			if (ImGui::MenuItem("Create Empty Entity")) {
				m_Context->CreateEntity("Empty Entity");
			}
			ImGui::EndPopup();
		}
		ImGui::End();
		// 判断当前点击的实体是否存在
		ImGui::Begin("Properties");
		if (m_SelectionContext) { // operator uint32_t() 的是const，不然不会调用operator bool(),而是调用uint32_t()
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
		// 要引入 Log头文件
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		// 若是被点击标记为选中状态|有下一级
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		// 完善UI1: 设置Tree的宽度为面板宽
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// 第一个参数是唯一ID 64的，
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked()) {
			m_SelectionContext = entity; // 记录当前点击的实体
		}
		// 右键实体-弹出菜单
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
		// 延后删除
		if (entityDeleted) {
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity) {
				m_SelectionContext = {};
			}
		}
	}
	// 完善UI：用模板+lambda替换冗余的ui绘制。类型2是参数传递推断出来的
	template<typename T,typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>()) {
			//ImGui::Separator();
			auto& component = entity.GetComponent<T>();
			// 为了定位+按钮在 最右边
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator(); // 水平风格线，测试放上面也行
			// 先绘制下三角
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			// 再绘制按钮
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f); // 设置下一个组件同一行，并且在父组件的最右边
			// 点击按钮-弹出菜单
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
				uiFunction(component);// 调用lambda函数
				// 展开树节点
				ImGui::TreePop();
			}
			// 延迟删除
			if (removeComponent) {
				entity.RemoveComponent<T>();
			}
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
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
				tag = std::string(buffer);
			}
		}
		// 完善UI：将添加组件按钮移到这里来
		// 在属性面板显示添加组件按钮，点击弹出菜单
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);// 说什么负数组件对齐右边
		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponent");// AddComponent只是id
		}
		if (ImGui::BeginPopup("AddComponent")) {
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<SpriteRendererComponent>("Sprite Renderer");
			DisplayAddComponentEntry<CircleRendererComponent>("Circle Renderer");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");
			ImGui::EndPopup();
		}
		ImGui::PopItemWidth();

		// 实体transform组件
		// 完善UI：模板类画组件的ui
		DrawComponent<TransformComponent>("Transform", entity, [](auto& tfc)
		{
			DrawVec3Control("Translation", tfc.Translation);
			glm::vec3 rotation = glm::degrees(tfc.Rotation);
			DrawVec3Control("Rotation", rotation); // 界面显示角度
			tfc.Rotation = glm::radians(rotation);
			DrawVec3Control("Scale", tfc.Scale, 1.0f);
		});
		// 摄像机组件
		// 完善UI：模板类画组件的ui
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
						// 设置摄像机的类型，重新计算投影矩阵
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
				float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV()); // 转换为角度
				if (ImGui::DragFloat("Vertical FOV", &verticalFov)) {
					camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov)); // 设置回弧度
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
		// 实体的脚本组件 mutable去除常量属性
		DrawComponent<ScriptComponent>("Script", entity, [entity](auto& component)mutable
		{
			bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);
			static char buffer[64];
			strcpy(buffer, component.ClassName.c_str());
			// 不存在这个实体，标红
			if (!scriptClassExists) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
			}
			if (ImGui::InputText("Class", buffer, sizeof(buffer))) {
				component.ClassName = buffer;
			}
			// 123：c#脚本属性
			// UUID->ScriptInstance->ScriptClass->fieldmap
			Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
			if (scriptInstance) {
				const auto& fields = scriptInstance->GetScriptClass()->GetFields();// 获取保存的属性名称
				for (const auto& [name, field] : fields)
				{
					if (field.Type == ScriptFieldType::Float) {
						float data = scriptInstance->GetFieldValue<float>(name);// 获取属性值
						if (ImGui::DragFloat(name.c_str(), &data)) {
							scriptInstance->SetFieldValue(name, data);// 设置属性值
						}
					}
				}
			}
			if (!scriptClassExists) {
				ImGui::PopStyleColor();
			}
		});
		// 实体SpriteRendererComponent组件		
		// 完善UI：模板类画组件的ui
		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
					// 判断是不是纹理？或者创建好后。创建的时候会有断言
					Ref<Texture2D> texture = Texture2D::Create(texturePath.string());
					if (texture->IsLoaded()) {
						component.Texture = texture;
					}
					else {
						HZ_WARN("Could not load texture {0}", texturePath.filename().string());
					}
				}
			}
			ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
		});
		DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](auto& component)
		{
			ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
			ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
		});
		// 物理
		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
		{
			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString)) {
				for (int i = 0; i < 2; i++) {
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
						currentBodyTypeString = bodyTypeStrings[i];
						component.Type = (Rigidbody2DComponent::BodyType)i;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
		});
		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
		{
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f, 1.0f);
		});
		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
		{
			ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
			ImGui::DragFloat("Radius", &component.Radius);
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});
	}
	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
	{
		if (!m_SelectionContext.HasComponent<T>()) {
			if (ImGui::MenuItem(entryName.c_str())) {
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}