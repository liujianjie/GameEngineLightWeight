#include "hzpch.h"
#include "OpenGLUniformBuffer.h"
#include <glad/glad.h>

namespace Hazel {
	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		// GPU创建缓冲区，并且返回缓冲区号
		glCreateBuffers(1, &m_RendererID);
		// 声明缓冲区的数据
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO:investigate usage hint
		// 将在glsl上设置的bingding 0号缓冲区与真正的缓冲区m_RendererID联系起来
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
		/*	 声明使用0号的uniform缓冲区
			layout(std140, binding = 0) uniform Camera
		*/
	}
	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}
	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		// 上传数据给m_RendererID号缓冲区吧，实则给GPU的bingding号缓冲区
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}
}

