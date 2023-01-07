#include "hzpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel {
	Ref<Texture2D> Hazel::Texture2D::Create(const std::string& path)
	{
		// 由于是shared_ptr,所以需要std::make_shared<OpenGLTexture2D>(path);初始化
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI:None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return std::make_shared<OpenGLTexture2D>(path);
		}
		HZ_CORE_ASSERT(false, "UnKnown RendererAPI!");
		return nullptr;
	}
}

