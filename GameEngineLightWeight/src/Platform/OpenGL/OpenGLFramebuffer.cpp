#include "hzpch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Hazel {
	static const uint32_t s_MaxFramebufferSize = 8192; // 16k

	// 工具类，为了给帧缓冲附加
	namespace Utils {
		static GLenum TextureTarget(bool multisampled) {
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}
		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count) {// outID 是vector的起始位置
			glCreateTextures(TextureTarget(multisampled), count, outID);;
		}
		static void BindTextures(bool multisampled, uint32_t id) {// id 是缓冲区id
			glBindTexture(TextureTarget(multisampled), id);
		}
		// 附加颜色纹理缓冲区
		static void AttachColorTextures(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index) {// id 是缓冲区id
			bool multisampled = samples > 1;
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else {
				// 旧的api吧
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
				// 这里写错过，将glTexParameteri 写错成glTextureParameteri！！
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			// 颜色纹理附加到帧缓冲：第二个参数重要，可以附加多个颜色纹理缓冲区！**
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}
		// 附加深度纹理缓冲
		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height){
			bool multisampled = samples > 1;
			if (multisampled) {
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
			}
			else {
				// 旧的api吧
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
				// 这里写错过，将glTexParameteri 写错成glTextureParameteri！！
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			// 深度缓冲附加到帧缓冲
			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}
		static bool IsDepthFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}
			return false;
		}
		static GLenum FramebufferTextureFormatToGLenum(FramebufferTextureFormat format) {
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:
				return GL_RGBA8;
			case FramebufferTextureFormat::RED_INTEGER:
				return GL_RED_INTEGER;
			}
			HZ_CORE_ASSERT(false);
			return 0;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		:m_Specification(spec)
	{
		// 传入要创建的
		for (auto spec : m_Specification.Attachments.Attachments) {
			if (!Utils::IsDepthFormat(spec.TextureFormat)) {
				m_ColorAttachmentSpecifications.emplace_back(spec);
			}
			else {
				m_DepthAttachmentSpecification = spec;
			}
		}
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);// 帧缓冲
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());// 颜色纹理缓冲
		glDeleteTextures(1, &m_DepthAttachment);	// 深度缓冲
	}

	void OpenGLFramebuffer::Invalidate()
	{
//#if old_path
		// 如果已有帧缓冲，要删除
		if (m_RendererID) { // 根据缓冲区ID删除
			glDeleteFramebuffers(1, &m_RendererID);// 帧缓冲
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());// 颜色纹理缓冲
			glDeleteTextures(1, &m_DepthAttachment);	// 深度缓冲

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}
		// 1.创建帧缓冲
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); // 绑定这个帧缓冲

		bool multisample = m_Specification.Samples > 1;
		// Attachments
		if (m_ColorAttachmentSpecifications.size()) {
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			// 2.1创建纹理。m_ColorAttachments.data()是地址，可以创建多个缓冲区
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());
			for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
				// 2.2绑定纹理
				Utils::BindTextures(multisample, m_ColorAttachments[i]);
				// 1.1纹理附加到帧缓冲
				switch (m_ColorAttachmentSpecifications[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
						Utils::AttachColorTextures(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
						break;
					case FramebufferTextureFormat::RED_INTEGER:
						Utils::AttachColorTextures(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
						break;
				}
			}
		}
		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None) {
			// 2.1创建深度缓冲
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			// 2.2绑定深度缓冲
			Utils::BindTextures(multisample, m_DepthAttachment);
			// 1.2深度模板缓冲纹理附加到帧缓冲中
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
				break;
			}
		}
		// 新加的！！！指定要Draw的颜色缓冲区列表
		if (m_ColorAttachments.size() > 1) {
			HZ_CORE_ASSERT(m_ColorAttachments.size() <= 4);
			// 这里id对应上面，颜色纹理附加到帧缓冲的ID
			GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2 ,GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}else if(m_ColorAttachments.empty()){
			// 只有深度缓冲
			glDrawBuffer(GL_NONE);
		}
		HZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "帧缓冲未创建完成");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);// 取消绑定这个帧缓冲,以免不小心渲染到错误的帧缓冲上，比如深度、模板缓冲不会渲染到这里
//#endif
#if old_path
		// 如果已有帧缓冲，要删除
		if (m_RendererID) {
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteFramebuffers(1, &m_ColorAttachment);
			glDeleteFramebuffers(1, &m_DepthAttachment);
		}
		// 1.创建帧缓冲
		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID); // 绑定这个帧缓冲

		// 2.1创建纹理
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);;
		// 2.2绑定纹理
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
		// 旧的api吧
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		// 这里写错过，将glTexParameteri 写错成glTextureParameteri！！
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 1.1纹理附加到帧缓冲
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

		// 3.创建深度模板缓冲纹理附加到帧缓冲中
		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);;
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
		// 新api吧
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

		// 1.2深度模板缓冲纹理附加到帧缓冲中
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		HZ_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "帧缓冲未创建完成");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);// 取消绑定这个帧缓冲,以免不小心渲染到错误的帧缓冲上，比如深度、模板缓冲不会渲染到这里
#endif
	}
	void Hazel::OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void Hazel::OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);// 取消绑定这个帧缓冲,以免不小心渲染到错误的帧缓冲上
	}
	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
			HZ_CORE_WARN("试图将frambuffer的大小设为{0} {1}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();// 重新生成
	}
	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		HZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData = -1;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}
	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		HZ_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::FramebufferTextureFormatToGLenum(spec.TextureFormat), GL_INT, &value);
	}
}
