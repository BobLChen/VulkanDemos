#version 450

layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;
layout (set = 0, binding = 2) uniform sampler2D samplerSpecularMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

float _CutOff = 0.57;
vec4  _MatColor = vec4(1, 0.35, 0.25, 1);
float _ScatterIntensity = 0.9;
float _CurvatureScale = 0.005;
float _Gloss = 100;
vec4  _SpecularColor = vec4(1.0, 1.0, 1.0, 1.0);
vec3  _lightColor = vec3(1.0, 1.0, 1.0);

vec3 lerp(vec3 a, vec3 b, float t)
{
	return a + (b - a) * t;
}

float GetIntegratedBRDF(float Omega, float K, float CutOff, float u, float v)
{
	float left      = 0.36 * cos(Omega * v) + 0.1;
	float inArea1   = max(sign(left - u), 0.0);
	float inArea3   = max(sign(K * (u - CutOff) - v), 0.0);
	float inArea2   = 1.0 - min(inArea1 + inArea3, 1.0);
	float value3    = 1;//0.5*u+0.38;
	float right     = v / K + CutOff;
	float amplitude = 0.5;//0.5 * (0.5 * right + 0.38);
	float omeg      = 3.1415926 / (right - left);
	float value2    = amplitude + amplitude * sin(omeg * (u - left - 0.5 * (right - left)));
	return (inArea2 * value2 + inArea3 * value3);
}

vec4 ShadeSkin(vec3 albedo, vec3 normal, vec3 specular, vec3 viewDir, vec3 lightDir)
{
	vec3 h       = normalize(lightDir + viewDir);
	float nh     = max(0, dot(normal, h));
	vec3 spec    = pow(nh, _Gloss) * specular;

	vec2 NdotL   = vec2(dot(normal, lightDir), dot(normal, lightDir));
	vec2 BRDFuv  = vec2(0.5 + 0.5 * NdotL.x, 1.0);
	float mask   = GetIntegratedBRDF(0.25 * 3.1415926, 10, _CutOff, BRDFuv.x, BRDFuv.y);
	vec3 tone    = _MatColor.rgb * GetIntegratedBRDF(_ScatterIntensity * 3.1415926, 10, 0.7, BRDFuv.x, BRDFuv.y);
	vec3 ramp    = vec3(0.3 * BRDFuv.x + 0.58);
	vec3 BRDFCol = lerp(tone, ramp, mask);

	vec3 diff_O  = _lightColor * BRDFCol * (0.5 + 0.5 * NdotL.x);
	BRDFuv  = vec2(0.5 + 0.5 * NdotL.y, 1.0);
	mask    = GetIntegratedBRDF(0.25 * 3.1415926, 10, _CutOff, BRDFuv.x, BRDFuv.y);
	tone    = _MatColor.rgb * GetIntegratedBRDF(_ScatterIntensity * 3.1415926, 10, 0.65, BRDFuv.x, BRDFuv.y);
	ramp    = vec3(0.3 * BRDFuv.x + 0.58);
	BRDFCol = lerp(tone, ramp, mask);

	vec3 diff_Blurred = _lightColor * BRDFCol * (0.5 + 0.5 * NdotL.y);
	vec3 diff = 0.5 * diff_O + 0.5 * diff_Blurred;

	vec4 c;
	c.rgb = albedo * diff + _lightColor * _SpecularColor.rgb * spec;
	c.a   = 1;
	
	return c;
}

void main() 
{
	vec3 lightDir = vec3(0, 0, -1);
	vec3 normal = normalize(inNormal);
	vec4 diffuse = texture(samplerColorMap, inUV0);
	vec4 specular = texture(samplerSpecularMap, inUV0);
    float ndl = max(dot(normal.rgb, lightDir), 0.0);

	vec3 viewDir = vec3(0, 1, 1);
	vec4 color = ShadeSkin(diffuse.rgb, normal, specular.rgb, viewDir, lightDir);

	outFragColor.rgb = color.rgb;
}