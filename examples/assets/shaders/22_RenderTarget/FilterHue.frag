#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float hueAdjust;
    float padding0;
    float padding1;
    float padding2;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float hueAdjust = filterParam.hueAdjust;
    
    vec4 kRGBToYPrime = vec4 (0.299, 0.587, 0.114, 0.0);
    vec4 kRGBToI = vec4 (0.595716, -0.274453, -0.321263, 0.0);
    vec4 kRGBToQ = vec4 (0.211456, -0.522591, 0.31135, 0.0);

    vec4 kYIQToR = vec4 (1.0, 0.9563, 0.6210, 0.0);
    vec4 kYIQToG = vec4 (1.0, -0.2721, -0.6474, 0.0);
    vec4 kYIQToB = vec4 (1.0, -1.1070, 1.7046, 0.0);

    vec4 color = texture(inputImageTexture, textureCoordinate);

    float YPrime = dot (color, kRGBToYPrime);
    float I = dot (color, kRGBToI);
    float Q = dot (color, kRGBToQ);

    float hue = atan (Q, I);
    float chroma = sqrt (I * I + Q * Q);

    hue += (-hueAdjust);

    Q = chroma * sin (hue);
    I = chroma * cos (hue);

    vec4 yIQ = vec4 (YPrime, I, Q, 0.0);
    color.r = dot (yIQ, kYIQToR);
    color.g = dot (yIQ, kYIQToG);
    color.b = dot (yIQ, kYIQToB);
    
    outFragColor = color;
}