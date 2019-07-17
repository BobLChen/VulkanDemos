#version 450

layout (binding = 1) uniform ParamBlock 
{
	vec3 lightDir;
    float curvature;

    vec3 lightColor;
    float exposure;

    vec2 curvatureScaleBias;
    float blurredLevel;
    float padding;
} params;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBiTangent;

layout (binding = 2) uniform sampler2D diffuseMap;
layout (binding = 3) uniform sampler2D normalMap;
layout (binding = 4) uniform sampler2D curvatureMap;
layout (binding = 5) uniform sampler2D preIntegratedMap;

layout (location = 0) out vec4 outFragColor;

vec3 EvaluateSSSDiffuseLight(vec3 normal, vec3 blurredNormal)
{
    float blurredNDL      = dot(blurredNormal, normalize(params.lightDir));
	float curvatureScaled = params.curvature * params.curvatureScaleBias.x + params.curvatureScaleBias.y;
	vec2 curvatureUV      = vec2(blurredNDL * 0.5 + 0.5, curvatureScaled);
	vec3 curvatureRGB     = texture(curvatureMap, curvatureUV).rgb * 0.5 - 0.25;
	vec3 normalFactor     = vec3(clamp(1.0 - blurredNDL, 0.0, 1.0));
	
    normalFactor *= normalFactor;

	vec3 normal0 = normalize(mix(normal, blurredNormal, 0.3 + 0.7 * normalFactor));
	vec3 normal1 = normalize(mix(normal, blurredNormal, normalFactor));
	float ndl0   = clamp(dot(normal0, normalize(params.lightDir)), 0.0, 1.0);
	float ndl1   = clamp(dot(normal1, normalize(params.lightDir)), 0.0, 1.0);
	vec3 ndlRGB  = vec3(clamp(blurredNDL, 0.0, 1.0), ndl0, ndl1);
	vec3 rgbSSS  = clamp(curvatureRGB + ndlRGB, 0.0, 1.0);

    return rgbSSS * params.lightColor;
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 bless  = step(vec3(0.04045), srgbIn.xyz);
	vec3 linOut = mix(srgbIn.xyz / vec3(12.92), pow((srgbIn.xyz + vec3(0.055)) / vec3(1.055), vec3(2.4)), bless);
	return vec4(linOut, srgbIn.w);;
}

vec3 Tonemap(vec3 rgb)
{
	rgb *= params.exposure;
	rgb = max(vec3(0), rgb - 0.004);
    rgb = (rgb * (6.2 * rgb + 0.5)) / (rgb * (6.2 * rgb + 1.7) + 0.06);
	return rgb;
}

void main() 
{
    // make tbn
    mat3 TBN = mat3(inTangent, inBiTangent, inNormal);

    // normal
    vec3 normal = texture(normalMap, inUV).xyz;
    normal = normalize(2.0 * normal - vec3(1.0, 1.0, 1.0));
    normal = TBN * normal;

    // blurredNormal
    vec3 blurredNormal = texture(normalMap, inUV, params.blurredLevel).xyz;
    blurredNormal = normalize(2.0 * blurredNormal - vec3(1.0, 1.0, 1.0));
    blurredNormal = TBN * blurredNormal;

    // diffuse
    vec3 diffuseColor = SRGBtoLINEAR(texture(diffuseMap, inUV)).xyz;

    // light
    vec3 diffuseLight = EvaluateSSSDiffuseLight(normal, blurredNormal);

    outFragColor = vec4(Tonemap(diffuseLight * diffuseColor), 1.0);
}