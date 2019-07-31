#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (binding = 0) uniform FilterParamBlock 
{
    float texelWidth;
    float texelHeight;
    vec2  padding0;
    
    mat4  convolutionMatrix;
} filterParam;

layout (location = 0) out vec2 textureCoordinate;
layout (location = 1) out vec2 leftTextureCoordinate;
layout (location = 2) out vec2 rightTextureCoordinate;

layout (location = 3) out vec2 topTextureCoordinate;
layout (location = 4) out vec2 topLeftTextureCoordinate;
layout (location = 5) out vec2 topRightTextureCoordinate;

layout (location = 6) out vec2 bottomTextureCoordinate;
layout (location = 7) out vec2 bottomLeftTextureCoordinate;
layout (location = 8) out vec2 bottomRightTextureCoordinate;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0);

    float texelWidth  = filterParam.texelWidth;
    float texelHeight = filterParam.texelHeight;

    vec2 widthStep               = vec2(texelWidth, 0.0);
    vec2 heightStep              = vec2(0.0, texelHeight);
    vec2 widthHeightStep         = vec2(texelWidth, texelHeight);
    vec2 widthNegativeHeightStep = vec2(texelWidth, -texelHeight);

    textureCoordinate            = inUV0.xy;
    leftTextureCoordinate        = inUV0.xy - widthStep;
    rightTextureCoordinate       = inUV0.xy + widthStep;

    topTextureCoordinate         = inUV0.xy - heightStep;
    topLeftTextureCoordinate     = inUV0.xy - widthHeightStep;
    topRightTextureCoordinate    = inUV0.xy + widthNegativeHeightStep;

    bottomTextureCoordinate      = inUV0.xy + heightStep;
    bottomLeftTextureCoordinate  = inUV0.xy - widthNegativeHeightStep;
    bottomRightTextureCoordinate = inUV0.xy + widthHeightStep;
}