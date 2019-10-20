#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inTangent;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBiTangent;
layout (location = 4) out vec3 outWorldPos;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 model33 = mat3(uboMVP.modelMatrix);
	mat3 normalMatrix = transpose(inverse(model33));

	vec3 normal  = normalize(normalMatrix * inNormal.xyz);
	vec3 tangent = normalize(normalMatrix * inTangent.xyz);
	
	outNormal    = normal;
	outTangent   = tangent;
	outBiTangent = cross(normal, tangent) * inTangent.w;

	outUV        = vec2(inUV0.x, 1.0 - inUV0.y);
	outWorldPos  = (uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0)).xyz;
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * vec4(outWorldPos, 1.0);
}