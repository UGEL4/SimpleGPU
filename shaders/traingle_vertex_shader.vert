#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5));
vec3 colors[3] 	  = vec3[](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
vec2 texCoord[3]  = vec2[](vec2(0.5, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0));

layout(location = 0) out vec3 frColor;
layout(location = 1) out vec2 outUV;
void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	frColor 	= colors[gl_VertexIndex];
	outUV		= texCoord[gl_VertexIndex];
}