#pragma once
#include "Camera.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/Timestep.h"
#include "Hazel/Events/MouseEvent.h"

#include <glm/glm.hpp>

namespace Hazel {
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		// 辅助代码
		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }
		
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		// 辅助代码

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		// 辅助代码
		const glm::vec3& GetPosition() const { return m_Position; }
		// 辅助代码

		glm::quat GetOrientation() const;

		// 辅助代码
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		// 辅助代码
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position = { 0.0f, 0.0f, 10.0f };	// 摄像机的位置
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f }; // 焦点的位置为原点

		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };// 记录当前鼠标位置，为了计算移动鼠标后焦点的位置

		float m_Distance = 10.0f;// 控制摄像机的z位置，为实现缩放效果
		float m_Pitch = 0.0f, m_Yaw = 0.0f; // m_Pitch x旋转 (仰俯视角)，m_Yaw y旋转

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};


}

