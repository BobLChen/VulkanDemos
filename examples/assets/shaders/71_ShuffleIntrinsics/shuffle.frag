#version 450 core

#extension GL_NV_shader_sm_builtins : require
#extension GL_ARB_shader_ballot : require
#extension GL_ARB_shader_group_vote : require

layout (location = 0) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

layout (binding = 0) uniform ParamBlock 
{
    vec4 type;
} uboParam;

void main() 
{
    outFragColor = vec4(0.0, 0.0, 0.0, 0.0);

    int type = int(uboParam.type.x);

    if (type == 0)
    {
        outFragColor = vec4(inUV0.x, inUV0.y, 0, 1);
    }
    else if (type == 1)
    {
        float s = float(gl_SubGroupInvocationARB) / float(32);
        outFragColor = vec4(s, s, s, s);
    }
    else if (type == 2)
    {
        float s = float(gl_WarpIDNV % 32) / float(32);
        outFragColor = vec4(s, s, s, s);
    }
    else if (type == 3)
    {
        float s = float(gl_SMIDNV % 32) / float(32);
        outFragColor = vec4(s, s, s, s);
    }
    else if (type == 4)
    {
        uint firstLaneId = readFirstInvocationARB(gl_SubGroupInvocationARB);

        if (firstLaneId == gl_SubGroupInvocationARB)
        {
            outFragColor = vec4(1, 1, 0, 0);
        }
        else
        {
            outFragColor = vec4(inUV0.x, inUV0.y, 0, 1);
        }
    }


}