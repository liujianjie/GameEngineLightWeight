#include "hzpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Hazel {
	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		:m_Specification(spec)
	{
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteFramebuffers(1, &m_ColorAttachment);
		glDeleteFramebuffers(1, &m_DepthAttachment);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		// �������֡���壬Ҫɾ��
		if (m_RendererID) {
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteFramebuffers(1, &m_ColorAttachment);
			glDeleteFramebuffers(1, &m_DepthAttachment);
		}
		// 1.����֡����
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); // �����֡����

		// 2.��������
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);;
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
		// �ɵ�api��
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		// ����д�������glTexParameteri д���glTextureParameteri����
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 1.1�����ӵ�֡����
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

		// 3.�������ģ�建�������ӵ�֡������
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);;
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
		// ��api��
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		// 1.2���ģ�建�������ӵ�֡������
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		HZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "֡����δ�������");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);// ȡ�������֡����,���ⲻС����Ⱦ�������֡�����ϣ�������ȡ�ģ�建�岻����Ⱦ������
	}
	void Hazel::OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void Hazel::OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);// ȡ�������֡����,���ⲻС����Ⱦ�������֡������
	}
	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();// ��������
	}
}
