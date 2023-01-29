#include "hzpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Hazel/Core/Timer.h"

namespace Hazel {

	namespace Utils {

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
				return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")
				return GL_FRAGMENT_SHADER;

			HZ_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return shaderc_glsl_vertex_shader;
			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
			}
			HZ_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
			}
			HZ_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_opengl.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_opengl.frag";
			}
			HZ_CORE_ASSERT(false);
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
			case GL_VERTEX_SHADER:    return ".cached_vulkan.vert";
			case GL_FRAGMENT_SHADER:  return ".cached_vulkan.frag";
			}
			HZ_CORE_ASSERT(false);
			return "";
		}


	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
		: m_FilePath(filepath)
	{
		HZ_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);

		//Compile(shaderSources);
		{
			Timer timer;
			CompileOrGetVulkanBinaries(shaderSources);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			HZ_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
		}

		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		HZ_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		CompileOrGetVulkanBinaries(sources);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGLShader::~OpenGLShader()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		HZ_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
			else
			{
				HZ_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			HZ_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		HZ_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			HZ_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			HZ_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		const bool optimize = true;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					HZ_CORE_ERROR(module.GetErrorMessage());
					HZ_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : shaderData)
			Reflect(stage, data);
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = false;
		if (optimize)
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					HZ_CORE_ERROR(module.GetErrorMessage());
					HZ_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			HZ_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		HZ_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		HZ_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
		HZ_CORE_TRACE("    {0} resources", resources.sampled_images.size());

		HZ_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			int memberCount = bufferType.member_types.size();

			HZ_CORE_TRACE("  {0}", resource.name);
			HZ_CORE_TRACE("    Size = {0}", bufferSize);
			HZ_CORE_TRACE("    Binding = {0}", binding);
			HZ_CORE_TRACE("    Members = {0}", memberCount);
		}
	}

	void OpenGLShader::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		HZ_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
{
	HZ_PROFILE_FUNCTION();

	GLuint program = glCreateProgram();
	/*
		std::vector<GLenum> glShaderIDs(shaderSources.size());
		glShaderIDs.reserve(2);
	*/
	std::array<GLenum, 2> glShaderIDs;
	int glShaderIDIndex = 0;
	for (auto& kv : shaderSources) {
		GLenum type = kv.first;
		const std::string& source = kv.second;

		// Create an empty vertex shader handle
		GLuint shader = glCreateShader(type);

		// Send the vertex shader source code to GL
		// Note that std::string's .c_str is NULL character terminated.
		const GLchar* sourceCStr = source.c_str();
		glShaderSource(shader, 1, &sourceCStr, 0);

		// Compile the vertex shader
		glCompileShader(shader);

		GLint isCompiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(shader);

			// Use the infoLog as you see fit.

			// In this simple program, we'll just leave
			HZ_CORE_ERROR("{0} ", infoLog.data());
			HZ_CORE_ASSERT(false, "shader 编译失败!");

			break;
		}

		// Attach our shaders to our program
		glAttachShader(program, shader);

		//glShaderIDs.push_back(shader);
		glShaderIDs[glShaderIDIndex++] = shader;
	}

	m_RendererID = program;

	// Link our program
	glLinkProgram(program);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(program);
		// Don't leak shaders either.
		for (auto id : glShaderIDs) {
			glDeleteShader(id);
		}
		// Use the infoLog as you see fit.
		// In this simple program, we'll just leave
		HZ_CORE_ERROR("{0} ", infoLog.data());
		HZ_CORE_ASSERT(false, "shader link failure!");
		return;
	}

	// Always detach shaders after a successful link.
	for (auto id : glShaderIDs) {
		glDetachShader(program, id);
	}
}
}

//#include "hzpch.h"
//#include "OpenGLShader.h"
//#include <filesystem>
//#include <fstream>
//#include <glad/glad.h>
//
//#include <glm/gtc/type_ptr.hpp>
//
//// vulkan的 SPIR-V东西
//#include <shaderc/shaderc.hpp>
//#include <spirv_cross/spirv_cross.hpp>
//#include <spirv_cross/spirv_glsl.hpp>
//
//#include "Hazel/Core/Timer.h"
//
//namespace Hazel {
//	namespace Utils {
//		static GLenum ShaderTypeFromString(const std::string& type) {
//			if (type == "vertex") {
//				return GL_VERTEX_SHADER;
//			}
//			if (type == "fragment" || type == "pixel") {
//				return GL_FRAGMENT_SHADER;
//			}
//			HZ_CORE_ASSERT(false, "不知道的shader类型");
//			return 0;
//		}
//		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage) {
//			switch (stage) {
//			case GL_VERTEX_SHADER: return shaderc_glsl_vertex_shader;
//			case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
//			}
//			HZ_CORE_ASSERT(false, "不知道的shader类型");
//			return (shaderc_shader_kind)0;
//		}
//		static const char* GLShaderStageToString(GLenum stage) {
//			switch (stage) {
//			case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
//			case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
//			}
//			HZ_CORE_ASSERT(false, "不知道的shader类型");
//			return nullptr;
//		}
//		static const char* GetCacheDirectory() {
//			// TODO: 确保这个路径存在
//			return "assets/cache/shader/opengl";
//		}
//		static void CreateCacheDirectoryIfNeeded() {
//			std::string cacheDirectory = GetCacheDirectory();
//			if (!std::filesystem::exists(cacheDirectory)) {
//				std::filesystem::create_directories(cacheDirectory); // 递归创建目录
//			}
//		}
//		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage) {
//			switch (stage) {
//			case GL_VERTEX_SHADER: return ".cached_opengl.vert";
//			case GL_FRAGMENT_SHADER: return ".cached_opengl.frag";
//			}
//			HZ_CORE_ASSERT(false, "不知道的shader类型");
//			return "";
//		}
//	}
//	
//	OpenGLShader::OpenGLShader(const std::string& filepath)
//		:m_FilePath(filepath)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		// 创建目录
//		Utils::CreateCacheDirectoryIfNeeded();
//
//		std::string source = ReadFile(filepath);
//		HZ_CORE_ASSERT(source.size(), "GLSL读取的字符串为空");
//		auto shaderSources = PreProcess(source);
//		//Compile(shaderSources);
//		{
//			Timer timer;
//			CompileOrGetVulkanBinaries(shaderSources);
//			CompileOrGetOpenGLBinaries();
//			CreateProgram();
//			HZ_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
//		}
//		/*
//			asserts\shaders\Texture.glsl
//			asserts/shaders/Texture.glsl
//			Texture.glsl
//		*/
//		auto lastSlash = filepath.find_last_of("/\\");// 字符串中最后一个正斜杠或者反斜杠
//		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
//		auto lastDot = filepath.rfind('.');
//		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
//		m_Name = filepath.substr(lastSlash, count);
//	}
//	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
//		:m_Name(name)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		std::unordered_map<GLenum, std::string> shaderSources;
//		shaderSources[GL_VERTEX_SHADER] = vertexSrc;
//		shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
//		//Compile(shaderSources);
//
//		CompileOrGetVulkanBinaries(shaderSources);
//		CompileOrGetOpenGLBinaries();
//		CreateProgram();
//	}
//	OpenGLShader::~OpenGLShader()
//	{
//		HZ_PROFILE_FUNCTION();
//
//		glDeleteProgram(m_RendererID);
//	}
//	std::string OpenGLShader::ReadFile(const std::string& filepath)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		std::string result;
//		//std::ifstream in(filepath, std::ios::in, std::ios::binary);// 二进制读取？为什么还是保持字符串的形式？
//		std::ifstream in(filepath, std::ios::in | std::ios::binary);// 二进制读取？为什么还是保持字符串的形式？
//		if (in) {
//			in.seekg(0, std::ios::end);			// 将指针放在最后面
//			size_t size = in.tellg();
//			if (size != -1) {
//				result.resize(in.tellg());			// 初始化string的大小, in.tellg()返回位置
//				in.seekg(0, std::ios::beg);			// in指回头部
//				in.read(&result[0], result.size());	// in读入放在result指向的内存中
//			}
//			else {
//				HZ_CORE_ERROR("Could not read from file '{0}'", filepath);
//			}
//		}
//		else {
//			HZ_CORE_ERROR("不能打开文件:{0}", filepath);
//		}
//		return result;
//	}
//	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		std::unordered_map<GLenum, std::string> shaderSources;
//
//		std::string typeToken = "#type";
//		size_t typeTokenLen = typeToken.size();
//		size_t findCurPos = source.find(typeToken, 0);
//		size_t findNextPos = findCurPos;
//		while (findNextPos != std::string::npos) {
//			size_t curlineEndPos = source.find_first_of("\r\n", findCurPos);///r/n写错为/r/n
//			HZ_CORE_ASSERT(curlineEndPos != std::string::npos, "解析shader失败" );
//			size_t begin = findCurPos + typeTokenLen + 1;
//
//			std::string type = source.substr(begin, curlineEndPos - begin);// 获取到是vertex还是fragment
//			HZ_CORE_ASSERT(Utils::ShaderTypeFromString(type), "无效的shader的类型	");
//
//			size_t nextLinePos = source.find_first_not_of("\r\n", curlineEndPos);
//			findNextPos = source.find(typeToken, nextLinePos);
//			// 获取到具体的shader代码
//			shaderSources[Utils::ShaderTypeFromString(type)] = source.substr(nextLinePos, findNextPos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
//
//			findCurPos = findNextPos;
//		}
//		return shaderSources;
//		/*
//			用find，而不是find_firtst_of，因为
//			find返回完全匹配的字符串的的位置；
//			find_first_of返回被查匹配字符串中某个字符的第一次出现位置。
//
//			std::string::npos是一个非常大的数
//			source.substr(0, source.size() + 10000)截取到从头到末尾，不会报错
//		*/
//	}
//	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
//	{
//		GLuint program = glCreateProgram();
//
//		shaderc::Compiler compiler;
//		shaderc::CompileOptions options;
//		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
//		const bool optimize = true;
//		if (optimize) {
//			// 优化：性能优先
//			options.SetOptimizationLevel(shaderc_optimization_level_performance);
//		}
//		// 生成二进制的缓存目录
//		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();
//
//		auto& shaderData = m_VulkanSPIRV;
//		shaderData.clear();
//		for (auto&& [stage, source] : shaderSources)
//		{
//			std::filesystem::path shaderFilePath = m_FilePath;
//			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));
//
//			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
//			// 缓存是否存在
//			if (in.is_open()) {// 存在打开加载
//				in.seekg(0, std::ios::end);
//				auto size = in.tellg();
//				in.seekg(0, std::ios::beg);
//
//				auto& data = shaderData[stage];// ?
//				data.resize(size / sizeof(uint32_t));
//				in.read((char*)data.data(), size);
//			}
//			else {
//				// 将Vulkan的glsl编译成SPIR-V二进制文件
//				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
//				if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
//					HZ_CORE_ERROR(module.GetErrorMessage());
//					HZ_CORE_ASSERT(false);
//				}
//
//				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());
//
//				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
//				if (out.is_open()) {
//					auto& data = shaderData[stage];
//					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
//					out.flush();
//					out.close();
//				}
//			}
//		}
//		for (auto&& [stage, data] : shaderData)
//			Reflect(stage, data);
//	}
	//void OpenGLShader::CompileOrGetOpenGLBinaries()
	//{
	//	auto& shaderData = m_OpenGLSPIRV;

	//	shaderc::Compiler compiler;
	//	shaderc::CompileOptions options;
	//	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
	//	const bool optimize = true;
	//	if (optimize) {
	//		// 优化：性能优先
	//		options.SetOptimizationLevel(shaderc_optimization_level_performance);
	//	}
	//	// 生成二进制的缓存目录
	//	std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

	//	shaderData.clear();
	//	m_OpenGLSourceCode.clear();
	//	for (auto&& [stage, spirv] : m_VulkanSPIRV)
	//	{
	//		std::filesystem::path shaderFilePath = m_FilePath;
	//		std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

	//		std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
	//		// 缓存是否存在
	//		if (in.is_open()) {// 存在打开加载
	//			in.seekg(0, std::ios::end);
	//			auto size = in.tellg();
	//			in.seekg(0, std::ios::beg);

	//			auto& data = shaderData[stage];// ?
	//			data.resize(size / sizeof(uint32_t));
	//			in.read((char*)data.data(), size);
	//		}
	//		else {
	//			// 将Vulkan的glsl的SPIR-V二进制文件转换为OpenGL的glsl源文件字符串
	//			spirv_cross::CompilerGLSL glslCompiler(spirv);
	//			m_OpenGLSourceCode[stage] = glslCompiler.compile();
	//			auto& source = m_OpenGLSourceCode[stage];
	//			// 再将OpenGL的glsl源文件字符串转换为SPIR-V二进制文件，并且保存在本地作为缓存
	//			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
	//			if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
	//				HZ_CORE_ERROR(module.GetErrorMessage());
	//				HZ_CORE_ASSERT(false);
	//			}

	//			shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

	//			std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
	//			if (out.is_open()) {
	//				auto& data = shaderData[stage];
	//				out.write((char*)data.data(), data.size() * sizeof(uint32_t));
	//				out.flush();
	//				out.close();
	//			}
	//		}
	//	}
	//}
//	void OpenGLShader::CreateProgram()
//	{
//		GLuint program = glCreateProgram();
//
//		std::vector<GLuint> shaderIDs;
//		for (auto&& [stage, spirv] : m_OpenGLSPIRV )
//		{
//			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
//			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
//			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
//			glAttachShader(program, shaderID);
//		}
//		glLinkProgram(program);
//
//		// Note the different functions here: glGetProgram* instead of glGetShader*.
//		GLint isLinked = 0;
//		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
//		if (isLinked == GL_FALSE)
//		{
//			GLint maxLength = 0;
//			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
//
//			// The maxLength includes the NULL character
//			std::vector<GLchar> infoLog(maxLength);
//			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
//
//			HZ_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());
//
//			// We don't need the program anymore.
//			glDeleteProgram(program);
//			// Don't leak shaders either.
//			for (auto id : shaderIDs) {
//				glDeleteShader(id);
//			}
//			// Use the infoLog as you see fit.
//			// In this simple program, we'll just leave
//			//HZ_CORE_ERROR("{0} ", infoLog.data());
//			//HZ_CORE_ASSERT(false, "shader link failure!");
//		}
//
//		// Always detach shaders after a successful link.
//		for (auto id : shaderIDs) {
//			glDetachShader(program, id);
//			glDeleteShader(id);
//		}
//		m_RendererID = program;
//	}
//	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
//	{
//		spirv_cross::Compiler compiler(shaderData);
//		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
//
//		HZ_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
//		HZ_CORE_TRACE("		{0} uniform buffers", resources.uniform_buffers.size());
//		HZ_CORE_TRACE("		{0} resources", resources.sampled_images.size());
//
//		HZ_CORE_TRACE("Uniform buffers:");
//		for (const auto& resource :resources.uniform_buffers)
//		{
//			const auto& bufferType = compiler.get_type(resource.base_type_id);
//			uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
//			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
//			int memberCount = bufferType.member_types.size();
//
//			HZ_CORE_TRACE("		{0}", resource.name);
//			HZ_CORE_TRACE("		size = {0}", bufferSize);
//			HZ_CORE_TRACE("		Binding = {0}", binding);
//			HZ_CORE_TRACE("		Members = {0}", memberCount);
//		}
//	}
//	
//
//	void OpenGLShader::Bind() const
//	{
//		HZ_PROFILE_FUNCTION();
//
//		glUseProgram(m_RendererID);
//	}
//	void OpenGLShader::UnBind() const
//	{
//		HZ_PROFILE_FUNCTION();
//
//		glUseProgram(0);
//	}
//	void OpenGLShader::SetInt(const std::string& name, int value)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		UploadUniformInt(name, value);
//	}
//	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
//	{
//		UploadUniformIntArray(name, values, count);
//	}
//	void OpenGLShader::SetFloat(const std::string& name, float value)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		UploadUniformFloat(name, value);
//	}
//	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		UploadUniformFloat3(name, value);
//	}
//	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		UploadUniformFloat4(name, value);
//	}
//	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
//	{
//		HZ_PROFILE_FUNCTION();
//
//		UploadUniformMat4(name, value);
//	}
//	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniform1i(location, value);
//	}
//	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniform1iv(location, count, values);
//	}
//	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniform1f(location, value);
//	}
//	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniform2f(location, value.x, value.y);
//	}
//	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniform3f(location, value.x, value.y, value.z);
//	}
//	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniform4f(location, value.x, value.y, value.z, value.w);
//	}
//	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
//	}
//	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
//	{
//		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
//		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
//	}
//}
//
////void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
////{
////	HZ_PROFILE_FUNCTION();
////
////	GLuint program = glCreateProgram();
////	/*
////		std::vector<GLenum> glShaderIDs(shaderSources.size());
////		glShaderIDs.reserve(2);
////	*/
////	std::array<GLenum, 2> glShaderIDs;
////	int glShaderIDIndex = 0;
////	for (auto& kv : shaderSources) {
////		GLenum type = kv.first;
////		const std::string& source = kv.second;
////
////		// Create an empty vertex shader handle
////		GLuint shader = glCreateShader(type);
////
////		// Send the vertex shader source code to GL
////		// Note that std::string's .c_str is NULL character terminated.
////		const GLchar* sourceCStr = source.c_str();
////		glShaderSource(shader, 1, &sourceCStr, 0);
////
////		// Compile the vertex shader
////		glCompileShader(shader);
////
////		GLint isCompiled = 0;
////		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
////		if (isCompiled == GL_FALSE)
////		{
////			GLint maxLength = 0;
////			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
////
////			// The maxLength includes the NULL character
////			std::vector<GLchar> infoLog(maxLength);
////			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
////
////			// We don't need the shader anymore.
////			glDeleteShader(shader);
////
////			// Use the infoLog as you see fit.
////
////			// In this simple program, we'll just leave
////			HZ_CORE_ERROR("{0} ", infoLog.data());
////			HZ_CORE_ASSERT(false, "shader 编译失败!");
////
////			break;
////		}
////
////		// Attach our shaders to our program
////		glAttachShader(program, shader);
////
////		//glShaderIDs.push_back(shader);
////		glShaderIDs[glShaderIDIndex++] = shader;
////	}
////
////	m_RendererID = program;
////
////	// Link our program
////	glLinkProgram(program);
////
////	// Note the different functions here: glGetProgram* instead of glGetShader*.
////	GLint isLinked = 0;
////	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
////	if (isLinked == GL_FALSE)
////	{
////		GLint maxLength = 0;
////		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
////
////		// The maxLength includes the NULL character
////		std::vector<GLchar> infoLog(maxLength);
////		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
////
////		// We don't need the program anymore.
////		glDeleteProgram(program);
////		// Don't leak shaders either.
////		for (auto id : glShaderIDs) {
////			glDeleteShader(id);
////		}
////		// Use the infoLog as you see fit.
////		// In this simple program, we'll just leave
////		HZ_CORE_ERROR("{0} ", infoLog.data());
////		HZ_CORE_ASSERT(false, "shader link failure!");
////		return;
////	}
////
////	// Always detach shaders after a successful link.
////	for (auto id : glShaderIDs) {
////		glDetachShader(program, id);
////	}
////}