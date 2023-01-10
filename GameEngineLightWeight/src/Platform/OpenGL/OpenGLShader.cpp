#include "hzpch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace Hazel {
	static GLenum ShaderTypeFromString(const std::string& type) {
		if (type == "vertex") {
			return GL_VERTEX_SHADER;
		}
		if (type == "fragment" || type == "pixel") {
			return GL_FRAGMENT_SHADER;
		}
		HZ_CORE_ASSERT(false, "��֪����shader����");
		return 0;
	}
	OpenGLShader::OpenGLShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		HZ_CORE_ASSERT(source.size(), "GLSL��ȡ���ַ���Ϊ��");
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);
		/*
			asserts\shaders\Texture.glsl
			asserts/shaders/Texture.glsl
			Texture.glsl
		*/
		auto lastSlash = filepath.find_last_of("/\\");// �ַ��������һ����б�ܻ��߷�б��
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);
	}
	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		:m_Name(name)
	{
		std::unordered_map<GLenum, std::string> shaderSources;
		shaderSources[GL_VERTEX_SHADER] = vertexSrc;
		shaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(shaderSources);
	}
	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		//std::ifstream in(filepath, std::ios::in, std::ios::binary);// �����ƶ�ȡ��Ϊʲô���Ǳ����ַ�������ʽ��
		std::ifstream in(filepath, std::ios::in | std::ios::binary);// �����ƶ�ȡ��Ϊʲô���Ǳ����ַ�������ʽ��
		if (in) {
			in.seekg(0, std::ios::end);			// ��ָ����������
			result.resize(in.tellg());			// ��ʼ��string�Ĵ�С, in.tellg()����λ��
			in.seekg(0, std::ios::beg);			// inָ��ͷ��
			in.read(&result[0], result.size());	// in�������resultָ����ڴ���
		}
		else {
			HZ_CORE_ERROR("���ܴ��ļ�:{0}", filepath);
		}
		return result;
	}
	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		std::string typeToken = "#type";
		size_t typeTokenLen = typeToken.size();
		size_t findCurPos = source.find(typeToken, 0);
		size_t findNextPos = findCurPos;
		while (findNextPos != std::string::npos) {
			size_t curlineEndPos = source.find_first_of("\r\n", findCurPos);///r/nд��Ϊ/r/n
			HZ_CORE_ASSERT(curlineEndPos != std::string::npos, "����shaderʧ��" );
			size_t begin = findCurPos + typeTokenLen + 1;

			std::string type = source.substr(begin, curlineEndPos - begin);// ��ȡ����vertex����fragment
			HZ_CORE_ASSERT(ShaderTypeFromString(type), "��Ч��shader������	");

			size_t nextLinePos = source.find_first_not_of("\r\n", curlineEndPos);
			findNextPos = source.find(typeToken, nextLinePos);
			// ��ȡ�������shader����
			shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, findNextPos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));

			findCurPos = findNextPos;
		}
		return shaderSources;
		/*
			��find��������find_firtst_of����Ϊ
			find������ȫƥ����ַ����ĵ�λ�ã�
			find_first_of���ر���ƥ���ַ�����ĳ���ַ��ĵ�һ�γ���λ�á�

			std::string::npos��һ���ǳ������
			source.substr(0, source.size() + 10000)��ȡ����ͷ��ĩβ�����ᱨ��
		*/
	}
	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		/*
			std::vector<GLenum> glShaderIDs(shaderSources.size());
			glShaderIDs.reserve(2);
		*/
		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto &kv : shaderSources) {
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
				HZ_CORE_ASSERT(false, "shader ����ʧ��!");

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
	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}
	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}
	void OpenGLShader::UnBind() const
	{
		glUseProgram(0);
	}
	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		UploadUniformFloat3(name, value);
	}
	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		UploadUniformFloat4(name, value);
	}
	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		UploadUniformMat4(name, value);
	}
	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
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
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}