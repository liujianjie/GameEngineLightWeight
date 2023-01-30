#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

namespace Hazel {

	// 
	extern const std::filesystem::path g_AssetPath = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		// Ϊ�˷�����һ��Ŀ¼
		// 1.��ǰĿ¼��= assetsĿ¼
		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath)) {
			// 2.�������˰�ť
			if (ImGui::Button("<-")) {
				// 3.��ǰĿ¼ = ��ǰĿ¼�ĸ�Ŀ¼
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}
		// Icon
		static float padding = 16.0f;
		static float thumbnailSize = 128.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount< 1) {
			columnCount = 1;
		}
		ImGui::Columns(columnCount, 0, false);

		// Ϊ�˱�����ǰĿ¼�µ��ļ����ļ���
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			// 1.�õ����ļ��л��ļ�path�ࡣ					���磺path = assets\cache\shader
			const auto& path = directoryEntry.path();
			// 2.�õ����ļ����assets�ļ��е����λ��path��	relativePath = cache\shader
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			// 3.��ȡ���ļ����ļ�����						filenameString = shader
			std::string filenameString = relativePath.filename().string();

			// On source items �϶�
			ImGui::PushID(filenameString.c_str());
			//ImGui::PushID(path.c_str());// ��֪��Ϊʲô����path.c_str()ȴû�ã�

			// Icon
			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));// alphaΪ0 ���ñ���Ϊ����ɫ
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0,1 }, { 1,0 });// 1:ID��2����С��3��4�����ϽǺ����½ǵ�uv����
			if (ImGui::BeginDragDropSource()) {
				// ���������϶�
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			// 4.1������ļ���Ŀ¼��˫�����ȥ���µ�ǰĿ¼
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (directoryEntry.is_directory()) {
					m_CurrentDirectory /= path.filename();
				}
			}
			ImGui::TextWrapped(filenameString.c_str());// ��������
			ImGui::NextColumn();// ��һ��

			ImGui::PopID();
		}
		// ���û�����
		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
		ImGui::End();
	}
}