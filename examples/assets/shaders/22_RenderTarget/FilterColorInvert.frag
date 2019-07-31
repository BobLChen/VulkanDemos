#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    outFragColor = vec4((1.0 - textureColor.rgb), textureColor.w);
}