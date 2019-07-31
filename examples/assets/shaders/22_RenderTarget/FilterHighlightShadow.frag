#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float shadows;
    float highlights;
    float padding0;
    float padding1;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float shadows = filterParam.shadows;
    float highlights = filterParam.highlights;
    
    vec3 luminanceWeighting = vec3(0.3, 0.3, 0.3);
    vec4 source = texture(inputImageTexture, textureCoordinate);
    float luminance = dot(source.rgb, luminanceWeighting);
    
    float shadow = clamp((pow(luminance, 1.0/(shadows+1.0)) + (-0.76)*pow(luminance, 2.0/(shadows+1.0))) - luminance, 0.0, 1.0);
    float highlight = clamp((1.0 - (pow(1.0-luminance, 1.0/(2.0-highlights)) + (-0.8)*pow(1.0-luminance, 2.0/(2.0-highlights)))) - luminance, -1.0, 0.0);
    vec3 result = vec3(0.0, 0.0, 0.0) + ((luminance + shadow + highlight) - 0.0) * ((source.rgb - vec3(0.0, 0.0, 0.0))/(luminance - 0.0));
    
    outFragColor = vec4(result.rgb, source.a);
}