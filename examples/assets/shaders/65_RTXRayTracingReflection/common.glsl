#extension GL_EXT_control_flow_attributes : require

struct RayPayloadInfo
{
	vec4 colorAndDistance;
	vec4 scatterDirection;
	uint randomSeed;
};

uint InitRandomSeed(uint val0, uint val1)
{
	uint v0 = val0, v1 = val1, s0 = 0;

	[[unroll]]
	for (uint n = 0; n < 16; n++) {
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}

	return v0;
}

uint RandomInt(inout uint seed)
{
    seed = 1664525 * seed + 1013904223;
    return seed;
}

float RandomFloat(inout uint seed)
{
	const uint one = 0x3f800000;
	const uint msk = 0x007fffff;
	return uintBitsToFloat(one | (msk & (RandomInt(seed) >> 9))) - 1;
}

vec2 RandomInUnitDisk(inout uint seed)
{
    uint count = 32;
    while (count > 0) {
        count -= 1;
        const vec2 p = 2 * vec2(RandomFloat(seed), RandomFloat(seed)) - 1;
        if (dot(p, p) < 1) {
			return p;
		}
    }

    return vec2(0, 0);
}

vec3 RandomInUnitSphere(inout uint seed)
{
    uint count = 32;
    while (count > 0) {
        count -= 1;
        const vec3 p = 2 * vec3(RandomFloat(seed), RandomFloat(seed), RandomFloat(seed)) - 1;
        if (dot(p, p) < 1) {
			return p;
		}
    }
    
    return vec3(0, 0, 0);
}
