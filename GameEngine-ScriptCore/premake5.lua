project "GameEngine-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	-- ��ȡ����.sln���ڵ�λ��
	--targetdir ("%{wks.location}/GameEngine-Editor/Resources/Scripts")
	--objdir ("%{wks.location}/GameEngine-Editor/Resources/Scripts/Intermediates")
	targetdir ("../GameEngine-Editor/Resources/Scripts")
	objdir ("../GameEngine-Editor/Resources/Scripts/Intermediates")

	files{
		"Source/**.cs",
		"Properties/**.cs"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"