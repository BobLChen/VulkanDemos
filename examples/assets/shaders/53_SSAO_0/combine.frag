#version 450

const float kContrast = 0.6;
const float kGeometryCoeff = 0.8;
const float kBeta = 0.002;

#define SAMPLE_COUNT    12
#define HALF_MAX        65504.0
#define HALF_MAX_MINUS1 65472.0
#define EPSILON         1.0e-4
#define PI              3.14159265359
#define TWO_PI          6.28318530718

#define INTENSITY       1.0
#define RADIUS          0.25

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec4 inRay;

layout (binding  = 1) uniform sampler2D colorTexture;
layout (binding  = 2) uniform sampler2D normalTexture;
layout (binding  = 3) uniform sampler2D depthTexture;

layout (location = 0) out vec4 outFragColor;

layout (binding = 4) uniform ParamBlock
{
	vec4 data;
    mat4 view;
    mat4 invView;
    mat4 proj;
} param;

vec4 zBufferParams;

float Linear01Depth( float z )
{
	return 1.0 / (zBufferParams.x * z + zBufferParams.y);
}

float GradientNoise(vec2 uv)
{
    vec2 size = vec2(1400, 900);
    uv = floor(uv * size);
    float f = dot(vec2(0.06711056, 0.00583715), uv);
    return fract(52.9829189f * fract(f));
}

float UVRandom(float u, float v)
{
    float f = dot(vec2(12.9898, 78.233), vec2(u, v));
    return fract(43758.5453 * sin(f));
}

vec2 CosSin(float theta)
{
    float sn = sin(theta);
    float cs = cos(theta);
    return vec2(cs, sn);
}

vec3 PickSamplePoint(vec2 uv, float index)
{
    float gn = GradientNoise(uv);
    float u = fract(UVRandom(0.0, index + uv.x * 1e-10) + gn) * 2.0 - 1.0;
    float theta = (UVRandom(1.0, index + uv.x * 1e-10) + gn) * TWO_PI;
    vec3 v = vec3(CosSin(theta) * sqrt(1.0 - u * u), u);
    float l = sqrt((index + 1.0) / SAMPLE_COUNT) * RADIUS;
    return v * l;
}

void main() 
{
    vec4 inColor  = texture(colorTexture,  inUV0);
    vec4 inNormal = texture(normalTexture, inUV0);
    vec4 inDepth  = texture(depthTexture,  inUV0);

    float zNear   = param.data.x;
	float zFar    = param.data.y;
	float xMaxFar = param.data.z;
	float yMaxFar = param.data.w;

	float zc0 = 1.0 - zFar / zNear;
	float zc1 = zFar / zNear;
	zBufferParams = vec4(zc0, zc1, zc0 / zFar, zc1 / zFar);
    
    vec3 norm_o   = mat3(param.view) * (inNormal.xyz * 2.0 - 1.0);
    float depth_o = Linear01Depth(inDepth.r);
    vec3 vpos_o   = inRay.xyz * depth_o;

    float ao = 0.0;
    for (int s = 0; s < SAMPLE_COUNT; ++s)
    {
        vec3 v_s1 = PickSamplePoint(inUV0, floor(1.0001 * s));
        v_s1 = faceforward(v_s1, -norm_o, v_s1);
        vec3 vpos_s1 = vpos_o + v_s1;

        // Reproject the sample point
        vec3 spos_s1 = (param.proj * vec4(vpos_s1, 1.0)).xyz;
        vec2 uv_s1_01 = (spos_s1.xy / vpos_s1.z + 1.0) * 0.5;
        uv_s1_01.y = 1.0 - uv_s1_01.y;

        // Depth at the sample point
        float depth_s1 = texture(depthTexture, uv_s1_01).r;
        depth_s1 = Linear01Depth(depth_s1);

        // Relative position of the sample point
        vec3 vpos_s2 = inRay.xyz * depth_s1;
        vec3 v_s2 = vpos_s2 - vpos_o;

        // Estimate the obscurance value
        float a1 = max(dot(v_s2, norm_o) - kBeta * depth_o, 0.0);
        float a2 = dot(v_s2, v_s2) + EPSILON;
        ao += a1 / a2;
    }

    ao *= RADIUS; // Intensity normalization

    // Apply other parameters.
    ao = pow(ao * INTENSITY / SAMPLE_COUNT, kContrast);

    outFragColor = inColor;
}