#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(set = 0, binding = 4) uniform ubo1 {mat4 inputPosition;}uo1;

vec2 positions[3] = vec2[](vec2(0.0, -0.5), vec2(-0.5, 0.5), vec2(0.5, 0.5));

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}