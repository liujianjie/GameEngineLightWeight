// Basic Texture Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;
layout(location = 5) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec3 LocalPosition; // ������Ϊ�˼�¼Բ��
	vec4 Color;
	float Thickness;
	float Fade;
};
layout(location = 0) out VertexOutput Output;
layout(location = 4) out flat int v_EntityID;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Color = a_Color;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;

	v_EntityID = a_EntityID;

	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec3 LocalPosition; // ������Ϊ�˼�¼Բ��
	vec4 Color;
	float Thickness;
	float Fade;
};

layout(location = 0) in VertexOutput Input;
layout(location = 4) in flat int v_EntityID;

void main()
{
	// ������Բ�ĵľ������
	/*
		�������ͣ�
		smoothstep(edge0, edge1, x);edge0 ����������ֵ�������½磻edge1 ����������ֵ�������Ͻ磻x �������ڲ�ֵ��Դ���롣��x > edg1ʱ������1����x < edg0ʱ������0����x��edg0��edg1֮��ʱ������x
		length(a); ��������a�ĳ���
		LocalPosition��Բ�ģ�0��0���ͱ�Ե�㣨0,1��˵��
		LocalPosition(0,0);Input.Thickness=1
			distance = 1 - 0 = 1;
			circle = smoothstep(0, 0, 1) = 0;
			cirle = cirle * smoothstep(1 + 0, 1, 1) = 0;
			���Ա�Ե������͸������ʾ
		LocalPosition(0,1);Input.Thickness=1
			distance = 1 - 1 = 0;
			circle = smoothstep(0, 0, 0) = 0;
			cirle = cirle * smoothstep(1 + 0, 1, 0) = 0;
			���Ա�Ե������͸������ʾ
	*/
	float distance = 1.0 - length(Input.LocalPosition);
	float circle = smoothstep(0.0, Input.Fade, distance);
	circle *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);
	if (circle == 0.0) {
		discard;
	}
	o_Color = Input.Color;
	o_Color.a *= circle;

	// EntityID
	o_EntityID = v_EntityID;
}