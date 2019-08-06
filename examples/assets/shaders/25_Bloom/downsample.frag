#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D diffuseTexture;

layout (binding  = 0) uniform FilterParam 
{
    vec4 size;
} param;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec4 color = texture(diffuseTexture, inUV0);
    float luminance = dot(color.rgb, W);
    outFragColor = vec4(vec3(luminance - param.size.w), color.a);
}