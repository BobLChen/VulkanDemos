#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec2 sampleDivisor = vec2(1.0 / 200.0, 1.0 / 320.0);
    vec2 samplePos = textureCoordinate - mod(textureCoordinate, sampleDivisor);
    vec4 color = texture(inputImageTexture, samplePos);

    vec4 colorCyan = vec4(85.0 / 255.0, 1.0, 1.0, 1.0);
    vec4 colorMagenta = vec4(1.0, 85.0 / 255.0, 1.0, 1.0);
    vec4 colorWhite = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 colorBlack = vec4(0.0, 0.0, 0.0, 1.0);

    float blackDistance = distance(color, colorBlack);
    float whiteDistance = distance(color, colorWhite);
    float magentaDistance = distance(color, colorMagenta);
    float cyanDistance = distance(color, colorCyan);

    vec4 finalColor;
    float colorDistance = min(magentaDistance, cyanDistance);
    colorDistance = min(colorDistance, whiteDistance);
    colorDistance = min(colorDistance, blackDistance);

    if (colorDistance == blackDistance) 
    {
        finalColor = colorBlack;
    } 
    else if (colorDistance == whiteDistance) 
    {
        finalColor = colorWhite;
    } 
    else if (colorDistance == cyanDistance) 
    {
        finalColor = colorCyan;
    } 
    else 
    {
        finalColor = colorMagenta;
    }

    outFragColor = finalColor;
}