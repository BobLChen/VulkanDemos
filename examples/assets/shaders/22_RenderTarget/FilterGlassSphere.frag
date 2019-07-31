#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 center;
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

    const vec3 lightPosition = vec3(-0.5, 0.5, 1.0);
    const vec3 ambientLightPosition = vec3(0.0, 0.0, 1.0);

    vec2 textureCoordinateToUse = vec2(textureCoordinate.x, (textureCoordinate.y * aspectRatio + 0.5 - 0.5 * aspectRatio));
    float distanceFromCenter = distance(center, textureCoordinateToUse);
    float checkForPresenceWithinSphere = step(distanceFromCenter, radius);
    
    distanceFromCenter = distanceFromCenter / radius;
    
    float normalizedDepth = radius * sqrt(1.0 - distanceFromCenter * distanceFromCenter);
    vec3 sphereNormal = normalize(vec3(textureCoordinateToUse - center, normalizedDepth));
    
    vec3 refractedVector = 2.0 * refract(vec3(0.0, 0.0, -1.0), sphereNormal, refractiveIndex);
    refractedVector.xy = -refractedVector.xy;
    
    vec3 finalSphereColor = texture(inputImageTexture, (refractedVector.xy + 1.0) * 0.5).rgb;
    
    float lightingIntensity = 2.5 * (1.0 - pow(clamp(dot(ambientLightPosition, sphereNormal), 0.0, 1.0), 0.25));
    finalSphereColor += lightingIntensity;
    
    lightingIntensity  = clamp(dot(normalize(lightPosition), sphereNormal), 0.0, 1.0);
    lightingIntensity  = pow(lightingIntensity, 15.0);
    finalSphereColor += vec3(0.8, 0.8, 0.8) * lightingIntensity;
    
    outFragColor = vec4(finalSphereColor, 1.0) * checkForPresenceWithinSphere;
}