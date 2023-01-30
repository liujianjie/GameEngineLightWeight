#include "hzpch.h"
#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

namespace Hazel {

	// 
	static const std::filesystem::path s_AssetPath = "assets";
	static const std::filesystem::path testPath = "test.txt";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(s_AssetPath)
	{
	}
	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");
		// 为了返回上一级目录
		// 1.当前目录！= assets目录
		if (m_CurrentDirectory != std::filesystem::path(s_AssetPath)) {
			// 2.如果点击了按钮
			if (ImGui::Button("<-")) {
				// 3.当前目录 = 当前目录的父目录
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}
		// 为了遍历当前目录下的文件和文件夹
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			// 1.得到子文件夹或文件path类。					比如：path = assets\cache\shader
			const auto& path = directoryEntry.path();
			// 2.得到子文件与的assets文件夹的相对位置path。	relativePath = cache\shader
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			// 3.获取子文件的文件名。						filenameString = shader
			std::string filenameString = relativePath.filename().string();
			// 4.1如果子文件是目录，设置按钮，点进去更新当前目录
			if (directoryEntry.is_directory()) {
				if (ImGui::Button(filenameString.c_str())) {
					m_CurrentDirectory /= path.filename();
				}
			}else{
				// 4.2如果子文件是文件，设置按钮，点进去打开
				if (ImGui::Button(filenameString.c_str())) {
				}
			}
		}
		ImGui::End();
	}
}