workspace "GameEngineLightWeight"
	architecture "x64"
	startproject "Sandbox"
	configurations{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "GameEngineLightWeight/vendor/GLFW/include"
IncludeDir["Glad"] = "GameEngineLightWeight/vendor/Glad/include"
IncludeDir["ImGui"] = "GameEngineLightWeight/vendor/imgui"

include "GameEngineLightWeight/vendor/GLFW"
include "GameEngineLightWeight/vendor/Glad"
include "GameEngineLightWeight/vendor/imgui"
project "GameEngineLightWeight"
	location "GameEngineLightWeight"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "hzpch.h"
	pchsource "GameEngineLightWeight/src/hzpch.cpp"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}"
	}
	links{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}
	filter "system:windows"
		cppdialect "C++17"	
		--staticruntime "On"
		systemversion "latest"

		defines{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
			"IMGUI_IMPL_OPENGL_LOADER_CUSTOM"
		}

		postbuildcommands{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

		filter "configurations:Debug"
			defines "HZ_DEBUG"
			runtime "Debug"
			symbols "On"

		filter "configurations:Release"
			defines "HZ_RELEASE"
			runtime "Release"
			symbols "On"

		filter "configurations:Dist"
			defines "HZ_DIST"
			runtime "Release"
			symbols "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"GameEngineLightWeight/vendor/spdlog/include",
		"GameEngineLightWeight/src"
	}

	links{
		"GameEngineLightWeight"
	}

	filter "system:windows"
		cppdialect "C++17"	
		--staticruntime "On"
		systemversion "latest"

		defines{
			"HZ_PLATFORM_WINDOWS"
		}

		filter "configurations:Debug"
			defines "HZ_DEBUG"
			runtime "Debug"
			symbols "On"

		filter "configurations:Release"
			defines "HZ_RELEASE"
			runtime "Release"
			symbols "On"

		filter "configurations:Dist"
			defines "HZ_DIST"
			runtime "Release"
			symbols "On"