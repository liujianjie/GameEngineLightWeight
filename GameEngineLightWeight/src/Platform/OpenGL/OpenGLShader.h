#pragma once

#include <string>
#include "Hazel/Renderer/Shader.h"
#include <glm/glm.hpp>
typedef unsigned int GLenum;
namespace Hazel {
	class OpenGLShader : public Shader
	{
	public:
		// 从文件读取
		OpenGLShader(const std::string& filepath);
		// 从字符串读取
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value)override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;	// 纹理槽数组
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Name; };

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);	// 纹理槽数组
		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);
		
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		// 从路径读取文件成字符串
		std::string ReadFile(const std::string& filepath);
		// *重点*，将字符串转换为map
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		// 编译shader
		//void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
		// 用SPIR-V编译
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);

	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name; // 一个材质实例，一个名称，用来存储到map中的，保证shader实例程序结束后才销毁

		// 存储的临时spir二进制文件
		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;
		// 存储的原始的opengl字符串
		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;
	};
}

