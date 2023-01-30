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
		// 为了返回上一级目录
		// 1.当前目录！= assets目录
		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath)) {
			// 2.如果点击了按钮
			if (ImGui::Button("<-")) {
				// 3.当前目录 = 当前目录的父目录
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

		// 为了遍历当前目录下的文件和文件夹
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			// 1.得到子文件夹或文件path类。					比如：path = assets\cache\shader
			const auto& path = directoryEntry.path();
			// 2.得到子文件与的assets文件夹的相对位置path。	relativePath = cache\shader
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			// 3.获取子文件的文件名。						filenameString = shader
			std::string filenameString = relativePath.filename().string();

			// On source items 拖动
			ImGui::PushID(filenameString.c_str());
			//ImGui::PushID(path.c_str());// 不知道为什么放入path.c_str()却没用！

			// Icon
			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));// alpha为0 设置背景为无颜色
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0,1 }, { 1,0 });// 1:ID，2：大小，3、4：左上角和右下角的uv坐标
			if (ImGui::BeginDragDropSource()) {
				// 这里设置拖动
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			// 4.1如果子文件是目录，双击点进去更新当前目录
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				if (directoryEntry.is_directory()) {
					m_CurrentDirectory /= path.filename();
				}
			}
			ImGui::TextWrapped(filenameString.c_str());// 包括文字
			ImGui::NextColumn();// 下一列

			ImGui::PopID();
		}
		// 设置滑动条
		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		ImGui::SliderFloat("Padding", &padding, 0, 32);
		ImGui::End();
	}
}