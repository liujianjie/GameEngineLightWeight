#include "ParticleSystem.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include <random>

class Random
{
public:
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}
	// 返回0-1的浮点数。
	static float Float()
	{
		// 随机生成一个 0-max 再除以 max即得，0-1的浮点数
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

private:
	static std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};
std::mt19937 Random::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution;


ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(1000);
}

// 更新一个个粒子的信息，控制粒子销毁
void ParticleSystem::OnUpdate(Hazel::Timestep ts)
{
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
			continue;

		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
			continue;
		}

		particle.LifeRemaining -= ts;
		particle.Position += particle.Velocity * (float)ts;
		particle.Rotation += 0.01f * ts;
	}
}
// 绘制一个个粒子
void ParticleSystem::OnRender(Hazel::OrthographicCamera& camera)
{
	Hazel::Renderer2D::BeginScene(camera);
	for (auto& particle : m_ParticlePool)
	{
		// 当粒子标记为不存活，则不再绘制
		if (!particle.Active)
			continue;

		// Fade away particles
		float life = particle.LifeRemaining / particle.LifeTime;
		glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
		//color.a = color.a * life;
		// 线性插值
		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		glm::vec3 position = { particle.Position.x, particle.Position.y, 0.2f };
		// 渲染 Rotation is radius
		Hazel::Renderer2D::DrawRotatedQuad(position, { size, size }, particle.Rotation, color);
	}
	Hazel::Renderer2D::EndScene();
}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = particleProps.Velocity;
	// 在x正负2个方向，和y正负2个方向上的重力加速度
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

	// Color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

	//m_PoolIndex = m_PoolIndex == 0 ? (m_ParticlePool.size() / 2) : --m_PoolIndex % m_ParticlePool.size();
	// 这里：由于m_ParticlePool.size();返回无符号整形，所以，-1 % 无符号整形为正数，但是不会回到999下标，只回到小于999的下标
	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	//std::cout << m_PoolIndex << std::endl;
}
