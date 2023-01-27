#pragma once

#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel {
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		// imgui渲染帧缓冲中的东西， 根据id获取哪一个来渲染
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { 
			HZ_CORE_ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; 
		}
		virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; };

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;
		/*
			FramebufferTextureSpecification是附加到帧缓冲中的格式 与 缓冲区本身的存储格式
			颜色缓冲区是本身的存储格式，深度缓冲区是附加到帧缓冲中的格式。
			但若是多重采样纹理，无论是颜色缓冲还是深度缓冲都是本身的存储格式
		*/ 
		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;// 因为可以有多个颜色纹理缓冲区
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		// 这是缓冲区ID
		std::vector<uint32_t> m_ColorAttachments;// 因为可以有多个颜色纹理缓冲区
		uint32_t m_DepthAttachment = 0;
		uint32_t m_ColorAttachment = 0;
	};
}
