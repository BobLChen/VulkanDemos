#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputDepth;

layout (location = 0) in vec2 outUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 color = subpassLoad(inputColor).rgb;
	float depth = subpassLoad(inputDepth).r;

	outFragColor = vec4(color, 1.0);
}