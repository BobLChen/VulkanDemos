#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4  center;
    float radius;
    float aspectRatio;
    float refractiveIndex;
    float padding;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec2 center = filterParam.center.xy;
    float radius = filterParam.radius;
    float aspectRatio = filterParam.aspectRatio;
    float refractiveIndex = filterParam.refractiveIndex;

    vec2 textureCoordinateToUse = vec2(textureCoordinate.x, (textureCoordinate.y * aspectRatio + 0.5 - 0.5 * aspectRatio));
    float distanceFromCenter = distance(center, textureCoordinateToUse);
    float checkForPresenceWithinSphere = step(distanceFromCenter, radius);
    
    distanceFromCenter = distanceFromCenter / radius;
    
    float normalizedDepth = radius * sqrt(1.0 - distanceFromCenter * distanceFromCenter);
    vec3 sphereNormal = normalize(vec3(textureCoordinateToUse - center, normalizedDepth));
    
    vec3 refractedVector = refract(vec3(0.0, 0.0, -1.0), sphereNormal, refractiveIndex);
    
    outFragColor = texture(inputImageTexture, (refractedVector.xy + 1.0) * 0.5) * checkForPresenceWithinSphere;     
}