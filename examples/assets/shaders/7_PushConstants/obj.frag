#version 450

layout (location = 0) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

layout(push_constant) uniform PushConsts {
    layout(offset = 0)  vec4 lightDir;
	layout(offset = 16) vec4 lightColor;
} pushConsts;

void main() 
{
    vec3 normal = normalize(inNormal);
    vec3 lightDir = pushConsts.lightDir.xyz;
    vec4 diffuse = dot(normal, lightDir) * pushConsts.lightColor + vec4(0.2);
    outFragColor = diffuse;
}