#include "hzpch.h"
#include "SubTexture2D.h"

namespace Hazel {
	// 只需要min和max二个二维向量可以组合成4个点
	Hazel::SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max)
		:m_Texture(texture)
	{
		m_TextCoords[0] = { min.x, min.y };
		m_TextCoords[1] = { max.x, min.y };
		m_TextCoords[2] = { max.x, max.y };
		m_TextCoords[3] = { min.x, max.y };
	}

	Ref<SubTexture2D> Hazel::SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize, const glm::vec2& spriteSize)
	{
		glm::vec2 min = { (coords.x * cellSize.x) / texture->GetWidth(),  (coords.y * cellSize.y) / texture->GetHeight() };
		glm::vec2 max = { ((coords.x + spriteSize.x) * cellSize.x) / texture->GetWidth(),  ((coords.y + spriteSize.y) * cellSize.y) / texture->GetHeight() };
		return CreateRef<SubTexture2D>(texture, min, max);
	}
}

