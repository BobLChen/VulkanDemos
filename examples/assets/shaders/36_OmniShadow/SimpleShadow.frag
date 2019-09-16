#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inLightDir;

layout (binding = 1) uniform LightMVPBlock 
{
	vec4 position;
    vec4 bias;
} lightParam;

layout (binding  = 2) uniform samplerCube shadowMap;

layout (location = 0) out vec4 outFragColor;

float CalcAttenuation(float range, float d)
{
    return 1.0 - smoothstep(range * 0.75, range, d);
}

void main() 
{
    vec4 diffuse  = vec4(1.0, 1.0, 1.0, 1.0);

    float dist    = length(inLightDir);
    vec3 lightDir = inLightDir / dist;
    float atten   = CalcAttenuation(lightParam.position.w, dist);
    
    diffuse.xyz  = dot(-lightDir, inNormal) * diffuse.xyz * atten; 
    outFragColor = diffuse;

    float depth  = texture(shadowMap, lightDir).r;
    float shadow = 1.0;
    shadow = (dist <= depth + lightParam.bias.x) ? 1.0 : 0.0;
    
    diffuse.xyz *= shadow;

    // diffuse.xyz  = vec3(dist / 25.0f);
    outFragColor = diffuse;
}