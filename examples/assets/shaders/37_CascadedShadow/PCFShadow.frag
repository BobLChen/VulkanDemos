#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inShadowCoord;

layout (binding = 1) uniform LightMVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec4 direction;
} lightMVP;

layout (binding = 3) uniform ShadowParamBlock 
{
	vec4 bias;
} shadowParam;

layout (binding  = 2) uniform sampler2D shadowMap;

layout (location = 0) out vec4 outFragColor;

const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338)
);

void main() 
{
    vec4 diffuse  = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 lightDir = normalize(lightMVP.direction.xyz);
    
    diffuse.xyz  = dot(lightDir, inNormal) * diffuse.xyz; 
    outFragColor = diffuse;

    float depth0 = inShadowCoord.z - shadowParam.bias.x;
    ivec2 texDim = textureSize(shadowMap, 0);
    vec2 texStep = vec2(1.0 / texDim.x, 1.0 / texDim.y);

    float shadow = 0.0;
    for (int i = 0; i < 25; ++i)
    {
        vec2 offset  = Poisson25[i] * texStep * shadowParam.bias.y;
        float depth1 = texture(shadowMap, inShadowCoord.xy + offset).r;
        shadow += depth0 >= depth1 ? 0.0 : 1.0;
    }
    shadow /= 25;
    
    diffuse.xyz *= shadow;
    outFragColor = diffuse;
}