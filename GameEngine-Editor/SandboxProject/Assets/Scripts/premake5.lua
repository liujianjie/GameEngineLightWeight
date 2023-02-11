local UnifyRootDir = "../../../.."

workspace "Sandbox"
	architecture "x86_64"
	startproject "Sandbox"
	configurations{
		"Debug",
		"Release",
		"Dist"
	}
	flags{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Sandbox"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Binaries")
	objdir ("Intermediates")

	files{
		"Source/**.cs",
		"Properies/**.cs"
	}
	links{
		"GameEngine-ScriptCore"
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

group "Unify"
	include (UnifyRootDir .. "/GameEngine-ScriptCore")
group ""