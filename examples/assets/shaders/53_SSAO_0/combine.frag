#version 450

const float kContrast = 0.6;
const float kGeometryCoeff = 0.8;
const float kBeta = 0.005;

#define SAMPLE_COUNT    25
#define EPSILON         0.0001
#define TWO_PI          6.28318530718

#define INTENSITY       4.0
#define RADIUS          0.25

layout (location = 0) in vec2 inUV0;

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
    uv = floor(uv * size.xy);
    float f = dot(vec2(0.06711056, 0.00583715), uv);
    return fract(52.9829189 * fract(f));
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

float SampleDepthNormal(vec2 uv, out vec3 normal)
{
    vec4 inNormal = texture(normalTexture, uv);
    inNormal.xyz  = normalize(inNormal.xyz * 2.0 - 1.0);
    normal = mat3(param.view) * (normalize(inNormal).xyz);
    return SampleDepth(uv);
}

void main() 
{
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

    // Parameters used in coordinate conversion
    vec2 p11_22 = vec2(param.proj[0][0], param.proj[1][1]);
    vec2 p13_31 = vec2(param.proj[0][2], param.proj[1][2]);

    // View space normal and depth
    vec3 norm_o;
    float depth_o = SampleDepthNormal(uv, norm_o);

    // Reconstruct the view-space position.
    vec3 vpos_o = ReconstructViewPos(uv, depth_o, p11_22, p13_31);

    float ao = 0.0;

    for (int s = 0; s < int(SAMPLE_COUNT); s++)
    {
        // Sample point
        // This 'floor(1.0001 * s)' operation is needed to avoid a NVidia shader issue. This issue
        // is only observed on DX11.
        vec3 v_s1 = PickSamplePoint(uv, floor(1.0001 * s));
        v_s1 = faceforward(v_s1, -norm_o, v_s1);

        vec3 vpos_s1 = vpos_o + v_s1;

        // Reproject the sample point
        vec3 spos_s1 = (param.proj * vec4(vpos_s1, 1.0)).xyz;
        vec2 uv_s1_01 = (spos_s1.xy / vpos_s1.z + 1.0) * 0.5;

        // Depth at the sample point
        float depth_s1 = SampleDepth(uv_s1_01);

        float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(depth_o - depth_s1));
        ao += (depth_s1 >= vpos_s1.z + 0.025 ? 1.0 : 0.0) * rangeCheck;     
        
        // // Relative position of the sample point
        // vec3 vpos_s2 = ReconstructViewPos(uv_s1_01, depth_s1, p11_22, p13_31);
        // vec3 v_s2 = vpos_s2 - vpos_o;

        // // Estimate the obscurance value
        // float a1 = max(dot(v_s2, norm_o) - kBeta * depth_o, 0.0);
        // float a2 = dot(v_s2, v_s2) + EPSILON;
        // ao += a1 / a2;
    }

    ao /= SAMPLE_COUNT;

    // ao *= RADIUS; // Intensity normalization

    // Apply other parameters.
    // ao = pow(ao * INTENSITY / SAMPLE_COUNT, kContrast);

    vec4 finnalColor;
    finnalColor.xyz = vec3(ao * INTENSITY);
    finnalColor.w = 1.0;

    outFragColor = finnalColor;
}