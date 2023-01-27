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

		// imgui��Ⱦ֡�����еĶ����� ����id��ȡ��һ������Ⱦ
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { 
			HZ_CORE_ASSERT(index < m_ColorAttachments.size()); return m_ColorAttachments[index]; 
		}
		virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; };

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;
		/*
			FramebufferTextureSpecification�Ǹ��ӵ�֡�����еĸ�ʽ �� ����������Ĵ洢��ʽ
			��ɫ�������Ǳ���Ĵ洢��ʽ����Ȼ������Ǹ��ӵ�֡�����еĸ�ʽ��
			�����Ƕ��ز���������������ɫ���廹����Ȼ��嶼�Ǳ���Ĵ洢��ʽ
		*/ 
		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;// ��Ϊ�����ж����ɫ��������
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		// ���ǻ�����ID
		std::vector<uint32_t> m_ColorAttachments;// ��Ϊ�����ж����ɫ��������
		uint32_t m_DepthAttachment = 0;
		uint32_t m_ColorAttachment = 0;
	};
}
