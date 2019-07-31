#version 450

const int GAUSSIAN_SAMPLES = 9;

layout (location = 0) in vec2 textureCoordinate;
layout (location = 1) in vec2 blurCoordinates[GAUSSIAN_SAMPLES];

layout (binding = 0) uniform FilterParamBlock 
{
    vec2  singleStepOffset;
    float distanceNormalizationFactor;
    float padding0;
} filterParam;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4  centralColor;
    float gaussianWeightTotal;
    vec4  sum;
    vec4  sampleColor;
    float distanceFromCentralColor;
    float gaussianWeight;

    float distanceNormalizationFactor = filterParam.distanceNormalizationFactor;
    
    centralColor             = texture(inputImageTexture, blurCoordinates[4]);
    gaussianWeightTotal      = 0.18;
    sum                      = centralColor * 0.18;
    
    sampleColor              = texture(inputImageTexture, blurCoordinates[0]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.05 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[1]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.09 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[2]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.12 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal      += gaussianWeight;
    sum                      += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[3]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.15 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[5]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.15 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[6]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.12 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[7]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.09 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    sampleColor              = texture(inputImageTexture, blurCoordinates[8]);
    distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
    gaussianWeight           = 0.05 * (1.0 - distanceFromCentralColor);
    gaussianWeightTotal     += gaussianWeight;
    sum                     += sampleColor * gaussianWeight;

    outFragColor = sum / gaussianWeightTotal;
}