workspace "GameEngineLightWeight"
	architecture "x64"
	startproject "GameEngine-Editor"
	configurations{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "GameEngineLightWeight/vendor/GLFW/include"
IncludeDir["Glad"] = "GameEngineLightWeight/vendor/Glad/include"
IncludeDir["ImGui"] = "GameEngineLightWeight/vendor/imgui"
IncludeDir["glm"] = "GameEngineLightWeight/vendor/glm"
IncludeDir["stb_image"] = "GameEngineLightWeight/vendor/stb_image"
IncludeDir["entt"] = "GameEngineLightWeight/vendor/entt/include"
IncludeDir["yaml_cpp"] = "GameEngineLightWeight/vendor/yaml-cpp/include" -- 用yaml_cpp下划线是因为"%{IncludeDir.yaml_cpp}"只认识_ 不认识-
IncludeDir["ImGuizmo"] = "GameEngineLightWeight/vendor/ImGuizmo" 
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["Box2D"] = "GameEngineLightWeight/vendor/Box2D/include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
--LibraryDir["VulkanSDK_Debug"] = "%{wks.location}/GameEngineLightWeight/vendor/VulkanSDK/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

group "Dependencies"
	include "GameEngineLightWeight/vendor/GLFW"
	include "GameEngineLightWeight/vendor/Glad"
	include "GameEngineLightWeight/vendor/imgui"
	include "GameEngineLightWeight/vendor/yaml-cpp"
	include "GameEngineLightWeight/vendor/Box2D"
group ""

project "GameEngineLightWeight"
	location "GameEngineLightWeight"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"	
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "hzpch.h"
	pchsource "GameEngineLightWeight/src/hzpch.cpp"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
	}
	defines{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}
	includedirs{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.Box2D}"
	}
	links{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"opengl32.lib",
		"Box2D"
	}
	-- imguizmo不使用编译头？ 没用 这句
	filter "files:%{prj.name}/vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines{
			"HZ_PLATFORM_WINDOWS",
			"HZ_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		filter "configurations:Debug"
			defines "HZ_DEBUG"
			runtime "Debug"
			symbols "on"

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}"
			}

		filter "configurations:Release"
			defines "HZ_RELEASE"
			runtime "Release"
			symbols "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}"
			}

		filter "configurations:Dist"
			defines "HZ_DIST"
			runtime "Release"
			symbols "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}"
			}

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"	
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"GameEngineLightWeight/vendor/spdlog/include",
		"GameEngineLightWeight/src",
		"GameEngineLightWeight/vendor",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}"
	}

	links{
		"GameEngineLightWeight"
	}

	filter "system:windows"
		systemversion "latest"

		defines{
			"HZ_PLATFORM_WINDOWS"
		}

		filter "configurations:Debug"
			defines "HZ_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "HZ_RELEASE"
			runtime "Release"
			symbols "on"

		filter "configurations:Dist"
			defines "HZ_DIST"
			runtime "Release"
			symbols "on"


project "GameEngine-Editor"
	location "GameEngine-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"	
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs{
		"GameEngineLightWeight/vendor/spdlog/include",
		"GameEngineLightWeight/src",
		"GameEngineLightWeight/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}"
	}

	links{
		"GameEngineLightWeight"
	}

	filter "system:windows"
		systemversion "latest"

		defines{
			"HZ_PLATFORM_WINDOWS"
		}

		filter "configurations:Debug"
			defines "HZ_DEBUG"
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines "HZ_RELEASE"
			runtime "Release"
			symbols "on"

		filter "configurations:Dist"
			defines "HZ_DIST"
			runtime "Release"
			symbols "on"