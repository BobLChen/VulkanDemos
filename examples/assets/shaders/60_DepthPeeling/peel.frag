#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inProj;

layout (binding = 2) uniform sampler2D peelMap;

layout (binding = 3) uniform ParamBlock 
{
    vec4 layer;
} uboParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    if (uboParam.layer.x < 1)
    {
        if (inProj.z > texture(peelMap, inProj.xy).r) {
            discard;
        }
    }
    else
    {
        if (texture(peelMap, inProj.xy).r > inProj.z) {
            discard;
        }
    }

    vec3 N = normalize(inNormal);
    vec3 L = normalize(vec3(0, 0, -1));
    float NdotL = dot(N, L);

    outFragColor = max(NdotL, 0.2) * vec4(1, 1, 1, 1.0);
    outFragColor.a = 0.4;

    gl_FragDepth = inProj.z + uboParam.layer.y;
}