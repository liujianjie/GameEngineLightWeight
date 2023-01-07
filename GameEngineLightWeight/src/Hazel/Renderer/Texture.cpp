#include "hzpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel {
	Ref<Texture2D> Hazel::Texture2D::Create(const std::string& path)
	{
		// ������shared_ptr,������Ҫstd::make_shared<OpenGLTexture2D>(path);��ʼ��
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI:None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}
		HZ_CORE_ASSERT(false, "UnKnown RendererAPI!");
		return nullptr;
	}
}

