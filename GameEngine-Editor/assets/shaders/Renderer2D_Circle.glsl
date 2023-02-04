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
	vec3 LocalPosition; // 输出这个为了不管处于任何世界坐标，相对于圆心的长度不变
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
	vec3 LocalPosition; // 输出这个为了记录圆心
	vec4 Color;
	float Thickness;
	float Fade;
};

layout(location = 0) in VertexOutput Input;
layout(location = 4) in flat int v_EntityID;

void main()
{
	/*
		函数解释：
		step(edge0, x); 当x > edge0，返回1，当x < edge0 返回0。阶梯函数
		smoothstep(edge0, edge1, x);当x > edg1时，返回1，当x < edg0时，返回0，当x在edg0和edg1之间时，返回x。平滑的阶梯函数
		length(a); 返回向量a的长度。sqrt(x*x, y*y);

		LocalPosition拿圆心(0, 0)、中心点(0, 0.5)、边缘点（0,1）说明
		Input.Thickness=1; Input.Fade=0
		LocalPosition(0, 0);
			distance = 1 - 0 = 1;
			circle = smoothstep(0, 0, 1) = 1;
			cirle = cirle * smoothstep(1 + 0, 1, 1) = 1;
			所以圆心像素显示
		LocalPosition(0, 0.5);
			distance = 1 - 0.5 = 0.5;
			circle = smoothstep(0, 0, 0.5) = 1;
			cirle = cirle * smoothstep(1 + 0, 1, 1) = 1;
			所以中间点像素显示
		LocalPosition(0, 1);
			distance = 1 - 1 = 0;
			circle = smoothstep(0, 0, 0) = 0;
			cirle = cirle * smoothstep(1 + 0, 1, 0) = 0;
			所以边缘的像素透明不显示
	*/
	float distance = 1.0 - length(Input.LocalPosition);// distance = 1-圆心的距离
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