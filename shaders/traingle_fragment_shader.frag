#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 1) uniform sampler texSamp; //static sampler

layout(location = 0) out vec4 outClor;
layout(location = 0) in vec3 frColor;
layout(location = 1) in vec2 outUV;
void main()
{
	//outClor = vec4(frColor, 1.0);
	outClor = texture(sampler2D(tex, texSamp), outUV);
}