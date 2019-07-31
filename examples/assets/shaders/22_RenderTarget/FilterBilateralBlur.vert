#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

const int GAUSSIAN_SAMPLES = 9;

layout (binding = 0) uniform FilterParamBlock 
{
    vec2  singleStepOffset;
    float distanceNormalizationFactor;
    float padding0;
} filterParam;

layout (location = 0) out vec2 textureCoordinate;
layout (location = 1) out vec2 blurCoordinates[GAUSSIAN_SAMPLES];

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0);

    textureCoordinate = inUV0;

    for (int i = 0; i < GAUSSIAN_SAMPLES; i++)
    {
        int multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));
        vec2 blurStep  = float(multiplier) * filterParam.singleStepOffset;
        blurCoordinates[i] = inUV0.xy + blurStep;
    }

}