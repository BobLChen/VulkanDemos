#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;
layout (binding  = 2) uniform sampler2D bloomTexture;
layout (binding  = 3) uniform sampler2D luminanceTexture;

layout (binding = 4) uniform ParamBlock 
{
    vec4 intensity;
} paramData;

layout (location = 0) out vec4 outFragColor;

float EyeAdaption(float lum)
{
	return mix(0.2, lum, 0.5);
}

vec3 ACESFilm(vec3 x)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;
	return (x * (A * x + B)) / (x * (C * x + D) + E);
}

void main() 
{
    vec4 originColor = texture(originTexture, inUV0);
    vec4 bloomColor  = texture(bloomTexture, inUV0);
    vec4 luminance  = texture(luminanceTexture, vec2(0.5, 0.5));
    
    vec4 finalColor  = originColor + bloomColor;

    float adaptedLumDest = 2 / (max(0.1, 1 + 10 * EyeAdaption(luminance.x)));
    finalColor.xyz = ACESFilm((adaptedLumDest * paramData.intensity.y * finalColor).xyz);

    outFragColor = finalColor;
}