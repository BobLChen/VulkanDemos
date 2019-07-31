#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float crossHatchSpacing;
    float lineWidth;
    float padding1;
    float padding2;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    float crossHatchSpacing = filterParam.crossHatchSpacing;
    float lineWidth = filterParam.lineWidth;
    float luminance = dot(texture(inputImageTexture, textureCoordinate).rgb, W);
    vec4 colorToDisplay = vec4(1.0, 1.0, 1.0, 1.0);
    if (luminance < 1.00)
    {
        if (mod(textureCoordinate.x + textureCoordinate.y, crossHatchSpacing) <= lineWidth)
        {
            colorToDisplay = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    if (luminance < 0.75)
    {
        if (mod(textureCoordinate.x - textureCoordinate.y, crossHatchSpacing) <= lineWidth)
        {
            colorToDisplay = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    if (luminance < 0.50)
    {
        if (mod(textureCoordinate.x + textureCoordinate.y - (crossHatchSpacing / 2.0), crossHatchSpacing) <= lineWidth)
        {
            colorToDisplay = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    if (luminance < 0.3)
    {
        if (mod(textureCoordinate.x - textureCoordinate.y - (crossHatchSpacing / 2.0), crossHatchSpacing) <= lineWidth)
        {
            colorToDisplay = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    outFragColor = colorToDisplay;
}