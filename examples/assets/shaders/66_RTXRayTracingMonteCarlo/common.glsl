#extension GL_EXT_control_flow_attributes : require

#define PI        3.14159265358979323
#define TWO_PI    6.28318530717958648

struct RayPayloadInfo
{
	vec4 colorAndDistance;
	vec4 scatterDirection;
	vec4 seedAndRandom;
};

float Rand(inout RayPayloadInfo info)
{
	info.seedAndRandom.xy -= vec2(info.seedAndRandom.z * info.seedAndRandom.w);
	return fract(sin(dot(info.seedAndRandom.xy, vec2(12.9898, 78.233))) * 43758.5453);
}