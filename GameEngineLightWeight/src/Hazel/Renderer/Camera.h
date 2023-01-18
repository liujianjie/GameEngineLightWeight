#pragma once
#include <glm/glm.hpp>
namespace Hazel {
	class Camera {
	public:
		Camera(const glm::mat4& projection)
			: m_Projection(projection){}
		const glm::mat4& GetProjection() const { return m_Projection; }
		// TOOD:��͸��ͶӰ
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}