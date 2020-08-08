#version 450


layout (location = 0) in vec2 inUV0;

layout (binding = 1) uniform sampler2D textureMap;

layout (location = 0) out vec4 outFragColor;

layout (binding = 2) uniform SDFBlock 
{
    vec4 param0;    // Distance Mark、Outline Distance Mark、Glow Distance Mark、Smooth Delta
    vec4 param1;    // Shadow Smooth、Glow Smooth、Shadow Offset X、Shadow Offset Y
    vec4 param2;    // type
    vec4 mainColor;
    vec4 outlineColor;
    vec4 glowColor;
    vec4 shadowColor;
} uboSDF;

void main() 
{
    float _DistanceMark         = uboSDF.param0.x;
    float _OutlineDistanceMark  = uboSDF.param0.y;
    float _GlowDistanceMark     = uboSDF.param0.z;
    float _SmoothDelta          = uboSDF.param0.w;
    float _ShadowSmoothDelta    = uboSDF.param1.x;
    float _GlowSmoothDelta      = uboSDF.param1.y;
    float _ShadowOffsetX        = uboSDF.param1.z;
    float _ShadowOffsetY        = uboSDF.param1.w;
    vec4 _MainColor             = uboSDF.mainColor;
    vec4 _OutlineColor          = uboSDF.outlineColor;
    vec4 _GlowColor             = uboSDF.glowColor;
    vec4 _ShadowColor           = uboSDF.shadowColor;
    float _Type                 = uboSDF.param2.x;

    float dist = texture(textureMap, inUV0).r;

    vec4 sdfCol;
    sdfCol.rgb = _MainColor.rgb;
    sdfCol.a   = smoothstep(_DistanceMark - _SmoothDelta, _DistanceMark + _SmoothDelta, dist);

    vec4 outlineCol;
    outlineCol.a   = smoothstep(_OutlineDistanceMark - _SmoothDelta, _OutlineDistanceMark + _SmoothDelta, dist);
    outlineCol.rgb = _OutlineColor.rgb;

    vec4 glowCol;
    glowCol.a   = smoothstep(_GlowDistanceMark - _GlowSmoothDelta, _GlowDistanceMark + _GlowSmoothDelta, dist);
    glowCol.rgb = _GlowColor.rgb;

    float shadowDistance = texture(textureMap, inUV0 + vec2(_ShadowOffsetX, _ShadowOffsetY)).r;
    float shadowAlpha    = smoothstep(_DistanceMark - _ShadowSmoothDelta, _DistanceMark + _ShadowSmoothDelta, shadowDistance);
    vec4 shadowCol       = vec4(_ShadowColor.rgb, _ShadowColor.a * shadowAlpha);
    
    if (_Type == 0)
    {
        outFragColor = sdfCol;
    }
    else if (_Type == 1)
    {
        outFragColor = mix(outlineCol, sdfCol, sdfCol.a);
    }
    else if (_Type == 2)
    {
        outFragColor = mix(glowCol, sdfCol, sdfCol.a);
    }
    else if (_Type == 3)
    {
        outFragColor = mix(shadowCol, sdfCol, sdfCol.a);
    }
    else
    {
        outFragColor = sdfCol;
    }
}