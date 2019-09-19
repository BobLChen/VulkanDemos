#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec4 originColor = texture(originTexture, inUV0);
    float bright = dot(originColor.xyz, W);
    if (originColor.x >= 1.0)
    {
        outFragColor = vec4(originColor.x - 1.0);
    }
    else
    {
        outFragColor = vec4(0, 0, 0, 1);
    }
}