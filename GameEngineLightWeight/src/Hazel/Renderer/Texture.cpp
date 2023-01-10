#include "hzpch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Hazel {
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI:None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(width, height);
		}
		HZ_CORE_ASSERT(false, "UnKnown RendererAPI!");
		return nullptr;
	}
	Ref<Texture2D> Hazel::Texture2D::Create(const std::string& path)
	{
		// 由于是shared_ptr,所以需要std::make_shared<OpenGLTexture2D>(path);初始化
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI:None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
		}
		HZ_CORE_ASSERT(false, "UnKnown RendererAPI!");
		return nullptr;
	}
}

