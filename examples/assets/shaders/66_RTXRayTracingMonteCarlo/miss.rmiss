#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) rayPayloadInNV RayPayloadInfo rayInfo;

void main()
{
    const float t = 0.5 * (normalize(gl_WorldRayDirectionNV).y + 1);
    const vec3 skyColor = mix(vec3(1.0), vec3(0.5, 0.7, 1.0), t) * 15.0;
    rayInfo.colorAndDistance = vec4(skyColor, -1);
}