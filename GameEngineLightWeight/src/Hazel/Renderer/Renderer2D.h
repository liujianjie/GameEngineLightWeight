#pragma once

#include "OrthographicCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "SubTexture2D.h"
#include "Hazel/Renderer/Camera.h"
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Scene/Components.h"
namespace Hazel {
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);// Caemra类没有视图矩阵，所以需传入transform计算视图矩阵
		static void BeginScene(const EditorCamera& camera);						// EditorCamera，可以直接获取投影视图矩阵，所以不需要transform
		static void BeginScene(const OrthographicCamera& camera);// TODO:REMOVE
		static void EndScene();
		static void Flush();

		// 源语
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// rotation is radius
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Transform包含position、size，SpriteRendererComponent包含olor、texture
		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

		// 真正绘制的方法- 上面8个 都得调用这两个
		// 不需要size，是因为size包含在transform中
		// 不带纹理
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		// 带纹理
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
		// 不带纹理
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);

		// Line
		static void DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID = -1);

		// 根据一点中心位置确定4个点的位置绘制rect
		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID = -1);
		// 根据实体的transform确定顶点位置再绘制
		static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);

		// 辅助函数
		static float GetLineWidth();
		static void SetLineWidth(float width);
		// 当前渲染的信息
		struct Statistics {
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		// 开始初始化批渲染
		static void StartBatch();
		// 内存不够为了分批渲染要做的drawcall绘制和重置
		static void NextBatch();// 1.先drawcall渲染当前 2.重置内存指针
	};
}