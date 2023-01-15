#include "hzpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

namespace Hazel {
	// ������ɫ����
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		HZ_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;
		/*������Ҫ1��������������������id������*/ // ��GL_TEXTURE_2D��д���GL_TEXTURE
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		/*����OpenGLm_RendererID������洢����rbg8λ����ߵĻ�����*/
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		/*����opengl��������Сʱ�����Թ���*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINE);
		/*����opengl������Ŵ�ʱ����Χ��ɫ��ƽ��ֵ����*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// �������곬��1��ȡ�Ĵ�ʩ
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	// ��������
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path):
		m_Path(path)
	{
		HZ_PROFILE_FUNCTION();

		int width, height, channels;
		 stbi_set_flip_vertically_on_load(1);// ���ô�ֱ��ת������OpenGL�Ǵ������µģ�����Ҫ����
		/*
		·���������귵�ش�С��ͨ��rgb��rbga�����ص��ַ���ָ��ָ��ľ��Ƕ�ȡ������ͼƬ���ݣ�
		*/
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;
		HZ_CORE_ASSERT(internalFormat & dataFormat, "ͼƬ��ʽ��֧��");
		/*������Ҫ1��������������������id������*/ // ��GL_TEXTURE_2D��д���GL_TEXTURE
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		/*����OpenGLm_RendererID������洢����rbg8λ����ߵĻ�����*/
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
		/*����opengl��������Сʱ�����Թ���*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINE);
		/*����opengl������Ŵ�ʱ����Χ��ɫ��ƽ��ֵ����*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// �������곬��1��ȡ�Ĵ�ʩ
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		/*ָ����ȡ�����򣬽�����ͼƬ���ݸ��ϴ�OpenGL��m_RendererID��һ�������Ǽ��𡣡���ɶ������*/
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
		/*������OpenGL������ͷţ����ɵ��ַ���*/
		stbi_image_free(data);
	}
	/*
	data ָ�������Ϊ����ľ������ݣ����ɫ:0xffffffff,���϶�Ӧ�������data��ָ��·���µ���������
	*/
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		HZ_CORE_ASSERT(size == m_Width * m_Height * bpp, "���ݴ�С�������С����");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width , m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		HZ_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}
}
