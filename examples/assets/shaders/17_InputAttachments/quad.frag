#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputDepth;

layout (binding = 2) uniform ParamBlock
{
	int attachmentIndex;
	vec3 padding;
} param;

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec4 inEye;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 color = subpassLoad(inputColor).rgb;
	float depth = subpassLoad(inputDepth).r;

	if (param.attachmentIndex == 0) {
		outFragColor = vec4(color, 1.0);
	} 
	else if (param.attachmentIndex == 1) {
		outFragColor = vec4(depth, depth, depth, 1.0);
	} 
	else if (param.attachmentIndex == 2) {
		vec3 worldPos = inEye.xyz * depth;
		outFragColor = vec4(normalize(worldPos), 1.0);
	} else {
		outFragColor = vec4(color, 1.0);
	}
	
}