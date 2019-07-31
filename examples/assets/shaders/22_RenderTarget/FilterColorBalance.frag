#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 shadowsShift;
    vec4 midtonesShift;
    vec4 highlightsShift;
    int  preserveLuminosity;
    int  padding0;
    int  padding1;
    int  padding2;
} filterParam;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

vec3 RGBToHSL(vec3 color)
{
    vec3 hsl    = vec3(0);
    float fmin  = min(min(color.r, color.g), color.b);
    float fmax  = max(max(color.r, color.g), color.b);
    float delta = fmax - fmin;

    hsl.z = (fmax + fmin) / 2.0;

    if (delta == 0.0)
    {
        hsl.x = 0.0;
        hsl.y = 0.0;
    }
    else
    {
        if (hsl.z < 0.5) {
            hsl.y = delta / (fmax + fmin);
        }
        else {
            hsl.y = delta / (2.0 - fmax - fmin);
        }

        float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
        float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
        float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

        if (color.r == fmax) {
            hsl.x = deltaB - deltaG;
        }
        else if (color.g == fmax) {
            hsl.x = (1.0 / 3.0) + deltaR - deltaB;
        }
        else if (color.b == fmax) {
            hsl.x = (2.0 / 3.0) + deltaG - deltaR;
        }
        if (hsl.x < 0.0) {
            hsl.x += 1.0;
        }
        else if (hsl.x > 1.0) {
            hsl.x -= 1.0;
        }
    }

    return hsl;
}

float HueToRGB(float f1,  float f2,  float hue)
{
    if (hue < 0.0) {
        hue += 1.0;
    }
    else if (hue > 1.0) {
        hue -= 1.0;
    }
        
    float res = 0.0;
    if ((6.0 * hue) < 1.0) {
        res = f1 + (f2 - f1) * 6.0 * hue;
    }
    else if ((2.0 * hue) < 1.0) {
        res = f2;
    }
    else if ((3.0 * hue) < 2.0) {
        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    }
    else {
        res = f1;
    }

    return res;
}

vec3 HSLToRGB(vec3 hsl)
{
    vec3 rgb;

    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z);
    }
    else
    {
        float f2;
        if (hsl.z < 0.5) {
            f2 = hsl.z * (1.0 + hsl.y);
        }
        else {
            f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
        }

        float f1 = 2.0 * hsl.z - f2;

        rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
        rgb.g = HueToRGB(f1, f2, hsl.x);
        rgb.b = HueToRGB(f1, f2, hsl.x - (1.0/3.0));
    }

    return rgb;
}

float RGBToL(vec3 color)
{
    float fmin = min(min(color.r, color.g), color.b);
    float fmax = max(max(color.r, color.g), color.b);
    return (fmax + fmin) / 2.0;
}

void main() 
{
    vec3 shadowsShift = filterParam.shadowsShift.xyz;
    vec3 midtonesShift = filterParam.midtonesShift.xyz;
    vec3 highlightsShift = filterParam.highlightsShift.xyz;
    int  preserveLuminosity = filterParam.preserveLuminosity;

    vec4 textureColor = texture(inputImageTexture, textureCoordinate);

    vec3 lightness = textureColor.rgb;

    const float a = 0.25;
    const float b = 0.333;
    const float scale = 0.7;

    vec3 shadows = shadowsShift * (clamp((lightness - b) / -a + 0.5, 0.0, 1.0) * scale);
    vec3 midtones = midtonesShift * (clamp((lightness - b) / a + 0.5, 0.0, 1.0) * clamp((lightness + b - 1.0) / -a + 0.5, 0.0, 1.0) * scale);
    vec3 highlights = highlightsShift * (clamp((lightness + b - 1.0) / a + 0.5, 0.0, 1.0) * scale);

    vec3 newColor = textureColor.rgb + shadows + midtones + highlights;
    newColor = clamp(newColor, 0.0, 1.0);

    if (preserveLuminosity != 0) 
    {
        vec3 newHSL = RGBToHSL(newColor);
        float oldLum = RGBToL(textureColor.rgb);
        textureColor.rgb = HSLToRGB(vec3(newHSL.x, newHSL.y, oldLum));
        outFragColor = textureColor;
    } else {
        outFragColor = vec4(newColor.rgb, textureColor.w);
    }

}