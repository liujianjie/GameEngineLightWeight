#include "hzpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

namespace Hazel {
	// 创建白色纹理
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		HZ_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;
		/*是纹理、要1个、生成纹理缓冲区返回id给变量*/ // 是GL_TEXTURE_2D，写错过GL_TEXTURE
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		/*告诉OpenGLm_RendererID的纹理存储的是rbg8位，宽高的缓冲区*/
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		/*告诉opengl，纹理缩小时用线性过滤*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINE);
		/*告诉opengl，纹理放大时用周围颜色的平均值过滤*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// 纹理坐标超过1采取的措施
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	// 加载纹理
	OpenGLTexture2D::OpenGLTexture2D(const std::string& path):
		m_Path(path)
	{
		HZ_PROFILE_FUNCTION();

		int width, height, channels;
		 stbi_set_flip_vertically_on_load(1);// 设置垂直翻转，由于OpenGL是从上往下的，所以要设置
		/*
		路径，加载完返回大小，通道rgb、rbga，返回的字符串指针指向的就是读取的纹理图片数据！
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
		HZ_CORE_ASSERT(internalFormat & dataFormat, "图片格式不支持");
		/*是纹理、要1个、生成纹理缓冲区返回id给变量*/ // 是GL_TEXTURE_2D，写错过GL_TEXTURE
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		/*告诉OpenGLm_RendererID的纹理存储的是rbg8位，宽高的缓冲区*/
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);
		/*告诉opengl，纹理缩小时用线性过滤*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINE);
		/*告诉opengl，纹理放大时用周围颜色的平均值过滤*/
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// 纹理坐标超过1采取的措施
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		/*指定截取子区域，将纹理图片数据给上传OpenGL。m_RendererID后一个参数是级别。。。啥东西？*/
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
		/*设置完OpenGL后可以释放，生成的字符串*/
		stbi_image_free(data);
	}
	/*
	data 指向的数据为纹理的具体数据，如白色:0xffffffff,与上对应，上面的data是指向路径下的纹理数据
	*/
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		HZ_CORE_ASSERT(size == m_Width * m_Height * bpp, "数据大小与纹理大小不符");
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
