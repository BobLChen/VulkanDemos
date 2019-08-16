#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inShadowCoord;

layout (binding  = 2) uniform sampler2D shadowMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse  = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 lightDir = vec3(-1, 1, 0);
    diffuse.xyz   = dot(lightDir, inNormal) * diffuse.xyz; 
    outFragColor  = diffuse;

    float depth0  = inShadowCoord.z - 0.01;
    float depth1  = texture(shadowMap, inShadowCoord.xy).r;
    float shadow  = 1.0;
    if (depth0 >= depth1) {
        shadow = 0.1;
    }

    diffuse.xyz *= shadow;

    outFragColor = diffuse;
}