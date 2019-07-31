#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec2 textureCoordinate;
layout (location = 1) out vec2 leftTextureCoordinate;
layout (location = 2) out vec2 rightTextureCoordinate;
layout (location = 3) out vec2 topTextureCoordinate;
layout (location = 4) out vec2 bottomTextureCoordinate;
layout (location = 5) out float centerMultiplier;
layout (location = 6) out float edgeMultiplier;

layout (binding = 0) uniform FilterParamBlock 
{
    float imageWidthFactor;
    float imageHeightFactor;
    float sharpness;
    float padding2;
} filterParam;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0);
    
    float sharpness = filterParam.sharpness;
    float imageWidthFactor = filterParam.imageWidthFactor;
    float imageHeightFactor = filterParam.imageHeightFactor;

    vec2 widthStep = vec2(imageWidthFactor, 0.0);
    vec2 heightStep = vec2(0.0, imageHeightFactor);
    
    textureCoordinate = inUV0.xy;
    leftTextureCoordinate = inUV0.xy - widthStep;
    rightTextureCoordinate = inUV0.xy + widthStep;
    topTextureCoordinate = inUV0.xy + heightStep;     
    bottomTextureCoordinate = inUV0.xy - heightStep;
    
    centerMultiplier = 1.0 + 4.0 * sharpness;
    edgeMultiplier = sharpness;
}