#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding = 0) uniform FilterParamBlock 
{
    float aspectRatio;
    float radius;
    float scale;
    float padding;
    vec4  center;
} filterParam;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float aspectRatio = filterParam.aspectRatio;
    float radius      = filterParam.radius;
    float scale       = filterParam.scale;
    vec2  center      = filterParam.center.xy;

    vec2 textureCoordinateToUse = vec2(textureCoordinate.x, (textureCoordinate.y * aspectRatio + 0.5 - 0.5 * aspectRatio));
    float dist = distance(center, textureCoordinateToUse);
    textureCoordinateToUse = textureCoordinate;
    if (dist < radius)
    {
        textureCoordinateToUse -= center;
        float percent = 1.0 - ((radius - dist) / radius) * scale;
        percent = percent * percent;
        textureCoordinateToUse = textureCoordinateToUse * percent;
        textureCoordinateToUse += center;
    }
    
    outFragColor = texture(inputImageTexture, textureCoordinateToUse ); 
}