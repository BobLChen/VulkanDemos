#ifndef BRDF
#define BRDF

float GTR2(float NDotH, float a)
{
	float a2 = a * a;
	float t = 1.0 + (a2 - 1.0) * NDotH * NDotH;
	return a2 / (PI * t * t);
}

float SchlickFresnel(float u)
{
	float m = clamp(1.0 - u, 0.0, 1.0);
	float m2 = m * m;
	return m2 * m2 * m;
}

float SmithG_GGX(float NDotv, float alphaG)
{
	float a = alphaG * alphaG;
	float b = NDotv * NDotv;
	return 1.0 / (NDotv + sqrt(a + b - a * b));
}

vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

#endif