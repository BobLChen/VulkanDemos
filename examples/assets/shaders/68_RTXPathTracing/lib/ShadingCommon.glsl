#ifndef SHADING_COMMON
#define SHADING_COMMON

float DielectricSpecularToF0(float specular)
{
	return 0.08f * specular;
}

// [Burley, "Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering"]
float DielectricF0ToIor(float f0)
{
	return 2.0f / (1.0f - sqrt(f0)) - 1.0f;
}

vec3 ComputeF0(float specular, vec3 baseColor, float metallic)
{
	return mix(DielectricSpecularToF0(specular).xxx, baseColor, metallic.xxx);
}

#endif