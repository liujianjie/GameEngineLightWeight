#include "hzpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include <glad/glad.h>

namespace Hazel {
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path):
		m_Path(path)
	{
		int width, height, channels;
		 stbi_set_flip_vertically_on_load(1);// ���ô�ֱ��ת������OpenGL�Ǵ������µģ�����Ҫ����
		/*
		·���������귵�ش�С��ͨ��rgb��rbga�����ص��ַ���ָ��ָ��ľ��Ƕ�ȡ������ͼƬ���ݣ�
		*/
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image");
		m_Width = width;
		m_Height = height;

		/*������Ҫ1��������������������id������*/ // ��GL_TEXTURE_2D��д���GL_TEXTURE
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		/*����OpenGLm_RendererID������洢����rbg8λ����ߵĻ�����*/
		glTextureStorage2D(m_RendererID, 1, GL_RGB8, m_Width, m_Height);
		/*����opengl��������Сʱ�����Թ���*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINE);
		/*����opengl������Ŵ�ʱ����Χ��ɫ��ƽ��ֵ����*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		/*ָ����ȡ�����򣬽�����ͼƬ���ݸ��ϴ�OpenGL��m_RendererID��һ�������Ǽ��𡣡���ɶ������*/
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data);
		/*������OpenGL������ͷţ����ɵ��ַ���*/
		stbi_image_free(data);
	}
	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}
