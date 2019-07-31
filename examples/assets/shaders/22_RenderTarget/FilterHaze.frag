#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float distance;
    float slope;
    float padding0;
    float padding1;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float distance = filterParam.distance;
    float slope    = filterParam.slope;
    
    vec4 color = vec4(1.0);
    float d = textureCoordinate.y * slope + distance;
    vec4 c = texture(inputImageTexture, textureCoordinate);
    c = (c - d * color) / (1.0 -d);
    outFragColor = c;
}