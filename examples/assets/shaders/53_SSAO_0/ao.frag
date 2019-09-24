#version 450

#define TWO_PI 6.28318530718

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D colorTexture;
layout (binding  = 2) uniform sampler2D normalTexture;
layout (binding  = 3) uniform sampler2D depthTexture;

layout (location = 0) out vec4 outFragColor;

layout (binding = 5) uniform ParamBlock
{
	vec4 data;
    vec4 data1;
    vec4 data2;
    mat4 view;
    mat4 invView;
    mat4 proj;
} param;

vec4 zBufferParams;

float Linear01Depth(float z)
{
	return 1.0 / (zBufferParams.x * z + zBufferParams.y);
}

float frac1(float v)
{
    return v -  floor(v);
}

float GradientNoise(vec2 uv)
{
    vec2 size = vec2(1400, 900);
    uv = floor(uv * size.xy);
    float f = dot(vec2(0.06711056, 0.00583715), uv);
    return frac1(52.9829189 * frac1(f));
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
    float l = sqrt((index + 1.0) / kSampleCount) * kRadius;
    return v * l;
}

vec3 ReconstructViewPos(vec2 uv, float depth, vec2 p11_22, vec2 p13_31)
{
    return vec3((uv * 2.0 - 1.0 - p13_31) / p11_22 * depth, depth);
}

float SampleDepth(vec2 uv)
{
    vec4 inDepth = texture(depthTexture,  uv);
    float d = Linear01Depth(inDepth.r);
    return d * param.data.y;
}

vec3 SampleNormal(vec2 uv)
{
    vec4 inNormal = texture(normalTexture, uv);
    inNormal.xyz  = inNormal.xyz * 2.0 - 1.0;
    inNormal.w    = 0.0;
    inNormal = param.view * inNormal;
    return normalize(inNormal.xyz);
}

void main() 
{
    const float kContrast       = param.data1.x;
    const float kGeometryCoeff  = param.data1.y;
    const float kBeta           = param.data1.z;
    const float kEpsilon        = param.data1.w;

    const int kSampleCount      = int(param.data2.x);
    const float kIntensity      = param.data2.y;
    const float kRadius         = param.data2.z;
    
    // buffer params
    float zNear   = param.data.x;
    float zFar    = param.data.y;
    float xMaxFar = param.data.z;
    float yMaxFar = param.data.w;
    float zc0 = 1.0 - zFar / zNear;
	float zc1 = zFar / zNear;
	zBufferParams = vec4(zc0, zc1, zc0 / zFar, zc1 / zFar);

    // uv
    vec2 uv = inUV0;

    // Reconstruct the view space position.
    vec2 p11_22 = vec2(param.proj[0][0], param.proj[1][1]);
    vec2 p13_31 = vec2(param.proj[0][2], param.proj[1][2]);

    // View space normal and depth
    vec3 normal = SampleNormal(uv);
    float depth = SampleDepth(uv);

    // Reconstruct the view-space position.
    vec3 viewPos = ReconstructViewPos(uv, depth, p11_22, p13_31);

    float occlusion = 0.0;
    for (int index = 0; index < int(kSampleCount); index++)
    {
        // Sample point
        // This 'floor(1.0001 * index)' operation is needed to avoid a NVidia shader issue. This issue
        // is only observed on DX11.
        vec3 offset = PickSamplePoint(uv, floor(1.0001 * index));
        offset = faceforward(offset, -normal, offset);

        vec3 sampleViewPos = viewPos + offset;

        // Reproject the sample point
        vec3 sampleProjPos = (param.proj * vec4(sampleViewPos, 1.0)).xyz;
        vec2 sampleUV = (sampleProjPos.xy / sampleViewPos.z + 1.0) * 0.5;

        // Depth at the sample point
        float sampleDepth = SampleDepth(sampleUV);

        // Relative position of the sample point
        vec3 realViewPos = ReconstructViewPos(sampleUV, sampleDepth, p11_22, p13_31);
        vec3 sampleDir = realViewPos - viewPos;
        
        // Estimate the obscurance value
        float a1 = max(dot(sampleDir, normal) - kBeta * depth, 0.0);
        float a2 = dot(sampleDir, sampleDir) + kEpsilon;
        occlusion += a1 / a2;
    }

    occlusion *= kRadius; // Intensity normalization

    // Apply other parameters.
    occlusion = pow(occlusion * kIntensity / kSampleCount, kContrast);

    vec4 finnalColor;
    finnalColor.xyz = vec3(occlusion);
    finnalColor.w   = 1.0;

    outFragColor = finnalColor;
}