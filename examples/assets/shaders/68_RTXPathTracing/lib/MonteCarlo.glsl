#ifndef MONTE_CARLO
#define MONTE_CARLO

float Square(float x)
{
	return x * x;
}

vec2 Square(vec2 x)
{
	return x * x;
}

vec3 Square(vec3 x)
{
	return x * x;
}

vec4 Square(vec4 x)
{
	return x * x;
}

float Pow2(float x)
{
	return x * x;
}

vec2 Pow2(vec2 x)
{
	return x * x;
}

vec3 Pow2(vec3 x)
{
	return x * x;
}

vec4 Pow2(vec4 x)
{
	return x * x;
}

float Pow3(float x)
{
	return x * x * x;
}

vec2 Pow3(vec2 x)
{
	return x * x * x;
}

vec3 Pow3(vec3 x)
{
	return x * x * x;
}

vec4 Pow3(vec4 x)
{
	return x * x * x;
}

float Pow4(float x)
{
	float xx = x * x;
	return xx * xx;
}

vec2 Pow4(vec2 x)
{
	vec2 xx = x * x;
	return xx * xx;
}

vec3 Pow4(vec3 x)
{
	vec3 xx = x * x;
	return xx * xx;
}

vec4 Pow4(vec4 x)
{
	vec4 xx = x * x;
	return xx * xx;
}

float Pow5(float x)
{
	float xx = x * x;
	return xx * xx * x;
}

vec2 Pow5(vec2 x)
{
	vec2 xx = x * x;
	return xx * xx * x;
}

vec3 Pow5(vec3 x)
{
	vec3 xx = x * x;
	return xx * xx * x;
}

vec4 Pow5(vec4 x)
{
	vec4 xx = x * x;
	return xx * xx * x;
}

float Pow6(float x)
{
	float xx = x * x;
	return xx * xx * xx;
}

vec2 Pow6(vec2 x)
{
	vec2 xx = x * x;
	return xx * xx * xx;
}

vec3 Pow6(vec3 x)
{
	vec3 xx = x * x;
	return xx * xx * xx;
}

vec4 Pow6(vec4 x)
{
	vec4 xx = x * x;
	return xx * xx * xx;
}

float rcp(float x)
{
    return 1.0 / x;
}

float rsqrt(float x)
{
    return 1.0 / sqrt(x);
}

float ClampedPow(float x, float y)
{
    return pow(max(abs(x), 0.000001f), y);
}

float saturate(float x)
{
    return clamp(x, 0.0, 1.0);
}

float Luminance(vec3 color)
{
    return max(dot(color, vec3(0.3, 0.59, 0.11)), 0.0001);
}

uint ReverseBits32(uint bits)
{
	bits = ( bits << 16) | ( bits >> 16);
	bits = ( (bits & 0x00ff00ff) << 8 ) | ( (bits & 0xff00ff00) >> 8 );
	bits = ( (bits & 0x0f0f0f0f) << 4 ) | ( (bits & 0xf0f0f0f0) >> 4 );
	bits = ( (bits & 0x33333333) << 2 ) | ( (bits & 0xcccccccc) >> 2 );
	bits = ( (bits & 0x55555555) << 1 ) | ( (bits & 0xaaaaaaaa) >> 1 );
	return bits;
}

mat3 GetTangentBasis(vec3 tangentZ)
{
	const float iSign = tangentZ.z >= 0 ? 1 : -1;

	const float a = -rcp(iSign + tangentZ.z);

	const float b = tangentZ.x * tangentZ.y * a;
	
	vec3 tangentX = { 1 + iSign * a * Pow2(tangentZ.x), iSign * b, -iSign * tangentZ.x };
	
	vec3 tangentY = { b,  iSign + a * Pow2(tangentZ.y), -tangentZ.y };

	return mat3(tangentX, tangentY, tangentZ);
}

vec3 TangentToWorld(vec3 vec, vec3 tangentZ)
{
    return GetTangentBasis(tangentZ) * vec;
}

vec3 WorldToTangent(vec3 vec, vec3 tangentZ)
{
    return vec * GetTangentBasis(tangentZ);
}

vec2 Hammersley(uint index, uint numSamples, uvec2 rand)
{
    float a0 = float(index) / numSamples;
    float a1 = float(rand.x & 0xffff);
	float e1 = fract(a0 + a1 / (1 << 16));
	float e2 = float(ReverseBits32(index) ^ rand.y) * 2.3283064365386963e-10;
	return vec2(e1, e2);
}

vec2 Hammersley16(uint index, uint numSamples, uvec2 rand)
{
    float a0 = float(index) / numSamples;
    float a1 = float(rand.x) * (1.0 / 65536.0);
	float e1 = fract(a0 + a1);
	float e2 = float((ReverseBits32(index) >> 16) ^ rand.y) * (1.0 / 65536.0);
	return vec2(e1, e2);
}

vec2 UniformSampleDisk(vec2 e)
{
	float theta = 2 * PI * e.x;
	float radius = sqrt(e.y);
	return radius * vec2(cos(theta), sin(theta));
}

vec2 UniformSampleDiskConcentric(vec2 e)
{
	vec2 p = 2 * e - 1;
	float radius;
	float phi;
	if (abs(p.x) > abs(p.y))
	{
		radius = p.x;
		phi = (PI / 4) * (p.y / p.x);
	}
	else
	{
		radius = p.y;
		phi = (PI / 2) - (PI / 4) * (p.x / p.y);
	}

	return vec2(radius * cos(phi), radius * sin(phi));
}

vec4 UniformSampleSphere(vec2 e)
{
	float u1 = e.x;
	float u2 = e.y;
	
	float z = 1.0 - 2.0 * u1;
	float r = sqrt(max(0.f, 1.0 - z * z));
	float phi = 2.0 * PI * u2;
	float x = r * cos(phi);
	float y = r * sin(phi);

	float pdf = 1.0 / (4 * PI);

	return vec4(x, y, z, pdf);
}

vec4 UniformSampleHemisphere(vec2 e)
{
	float phi = 2 * PI * e.x;
	float cosTheta = e.y;
	float sinTheta = sqrt(1 - cosTheta * cosTheta);

	vec3 h;
	h.x = sinTheta * cos(phi);
	h.y = sinTheta * sin(phi);
	h.z = cosTheta;

	float pdf = 1.0 / (2 * PI);

	return vec4(h, pdf);
}

vec4 CosineSampleHemisphere(vec2 e)
{
	float phi = 2 * PI * e.x;
	float cosTheta = sqrt(e.y);
	float sinTheta = sqrt(1 - cosTheta * cosTheta);

	vec3 h;
	h.x = sinTheta * cos(phi);
	h.y = sinTheta * sin(phi);
	h.z = cosTheta;

	float pdf = cosTheta * (1.0 /  PI);

	return vec4(h, pdf);
}

vec4 CosineSampleHemisphere(vec2 e, vec3 n) 
{
	vec3 h = UniformSampleSphere(e).xyz;
	h = normalize(n + h);

	float pdf = dot(h, n) * (1.0 /  PI);

	return vec4(h, pdf);
}

vec4 UniformSampleCone(vec2 e, float cosThetaMax)
{
	float phi = 2 * PI * e.x;
	float cosTheta = mix(cosThetaMax, 1, e.y);
	float sinTheta = sqrt(1 - cosTheta * cosTheta);

	vec3 l;
	l.x = sinTheta * cos(phi);
	l.y = sinTheta * sin(phi);
	l.z = cosTheta;

	float pdf = 1.0 / (2 * PI * (1 - cosThetaMax));

	return vec4(l, pdf);
}

float PowerHeuristic(float a, float b)
{
	float t = a * a;
	return t / (b * b + t);
}

vec3 CosineSampleHemisphere(float u1, float u2)
{
	float r = sqrt(u1);
	float phi = 2.0 * PI * u2;
	vec3 dir;
	dir.x = r * cos(phi);
	dir.y = r * sin(phi);
	dir.z = sqrt(max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));
	return dir;
}

#endif