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
		// Ϊ�˷�����һ��Ŀ¼
		// 1.��ǰĿ¼��= assetsĿ¼
		if (m_CurrentDirectory != std::filesystem::path(s_AssetPath)) {
			// 2.�������˰�ť
			if (ImGui::Button("<-")) {
				// 3.��ǰĿ¼ = ��ǰĿ¼�ĸ�Ŀ¼
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}
		// Ϊ�˱�����ǰĿ¼�µ��ļ����ļ���
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory)) {
			// 1.�õ����ļ��л��ļ�path�ࡣ					���磺path = assets\cache\shader
			const auto& path = directoryEntry.path();
			// 2.�õ����ļ����assets�ļ��е����λ��path��	relativePath = cache\shader
			auto relativePath = std::filesystem::relative(path, s_AssetPath);
			// 3.��ȡ���ļ����ļ�����						filenameString = shader
			std::string filenameString = relativePath.filename().string();
			// 4.1������ļ���Ŀ¼�����ð�ť�����ȥ���µ�ǰĿ¼
			if (directoryEntry.is_directory()) {
				if (ImGui::Button(filenameString.c_str())) {
					m_CurrentDirectory /= path.filename();
				}
			}else{
				// 4.2������ļ����ļ������ð�ť�����ȥ��
				if (ImGui::Button(filenameString.c_str())) {
				}
			}
		}
		ImGui::End();
	}
}