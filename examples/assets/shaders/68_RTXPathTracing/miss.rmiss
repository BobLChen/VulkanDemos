#version 460
#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "lib/Common.glsl"

layout(location = 0) rayPayloadInNV RayPayloadInfo rayInfo;

void main()
{
    rayInfo.hitT = -1.0;
}