#version 450

layout (location = 0) in vec3  inPosition;
layout (location = 1) in vec2  inUV0;
layout (location = 2) in vec3  inNormal;
layout (location = 3) in vec3  inSkinPack;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

#define MAX_BONES 64

layout (binding = 1) uniform BonesTransformBlock 
{
	mat2x4 	dualQuats[MAX_BONES];
	vec4 	debugParam;
} bonesData;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

ivec4 UnPackUInt32To4Byte(uint packIndex)
{
	uint idx0 = (packIndex >> 24) & 0xFF;
	uint idx1 = (packIndex >> 16) & 0xFF;
	uint idx2 = (packIndex >> 8)  & 0xFF;
	uint idx3 = (packIndex >> 0)  & 0xFF;
	return ivec4(idx0, idx1, idx2, idx3);
}

ivec2 UnPackUInt32To2Short(uint packIndex)
{
	uint idx0 = (packIndex >> 16) & 0xFFFF;
	uint idx1 = (packIndex >> 0)  & 0xFFFF;
	return ivec2(idx0, idx1);
}

mat4x4 DualQuat2Matrix(vec4 real, vec4 dual) {
	
	float len2 = dot(real, real);
	float qx = real.x;
	float qy = real.y;
	float qz = real.z;
	float qw = real.w;

	float tx = dual.x;
	float ty = dual.y;
	float tz = dual.z;
	float tw = dual.w; 
	
	mat4x4 matrix;
	matrix[0][0] = qw * qw + qx * qx - qy * qy - qz * qz;
	matrix[1][0] = 2 * qx * qy - 2 * qw * qz;
	matrix[2][0] = 2 * qx * qz + 2 * qw * qy;
	matrix[0][1] = 2 * qx * qy + 2 * qw * qz;
	matrix[1][1] = qw * qw + qy * qy - qx * qx - qz * qz;
	matrix[2][1] = 2 * qy * qz - 2 * qw * qx;
	matrix[0][2] = 2 * qx * qz - 2 * qw * qy;
	matrix[1][2] = 2 * qy * qz + 2 * qw * qx;
	matrix[2][2] = qw * qw + qz * qz - qx * qx - qy * qy;

	matrix[3][0] = -2 * tw * qx + 2 * qw * tx - 2 * ty * qz + 2 * qy * tz;
	matrix[3][1] = -2 * tw * qy + 2 * tx * qz - 2 * qx * tz + 2 * qw * ty;
	matrix[3][2] = -2 * tw * qz + 2 * qx * ty + 2 * qw * tz - 2 * tx * qy;

	matrix[0][3] = 0;
	matrix[1][3] = 0;
	matrix[2][3] = 0;
	matrix[3][3] = len2;

	matrix /= len2;

	return matrix;
}

vec3 DualQuatTransformPosition(mat2x4 dualQuat, vec3 position)
{
	float len = length(dualQuat[0]);
	dualQuat /= len;
	
	vec3 result = position.xyz + 2.0 * cross(dualQuat[0].xyz, cross(dualQuat[0].xyz, position.xyz) + dualQuat[0].w * position.xyz);
	vec3 trans  = 2.0 * (dualQuat[0].w * dualQuat[1].xyz - dualQuat[1].w * dualQuat[0].xyz + cross(dualQuat[0].xyz, dualQuat[1].xyz));
	result += trans;

	return result;
}

vec3 DualQuatTransformVector(mat2x4 dualQuat, vec3 vector)
{
	return vector + 2.0 * cross(dualQuat[0].xyz, cross(dualQuat[0].xyz, vector) + dualQuat[0].w * vector);
}

void main() 
{
	// skin info
	ivec4 skinIndex   = UnPackUInt32To4Byte(uint(inSkinPack.x));
	ivec2 skinWeight0 = UnPackUInt32To2Short(uint(inSkinPack.y));
	ivec2 skinWeight1 = UnPackUInt32To2Short(uint(inSkinPack.z));
	vec4  skinWeight  = vec4(skinWeight0 / 65535.0, skinWeight1 / 65535.0);

	// dual quats
	mat2x4 dualQuat0 = bonesData.dualQuats[skinIndex.x];
	mat2x4 dualQuat1 = bonesData.dualQuats[skinIndex.y];
	mat2x4 dualQuat2 = bonesData.dualQuats[skinIndex.z];
	mat2x4 dualQuat3 = bonesData.dualQuats[skinIndex.w];
	
	if (dot(dualQuat0[0], dualQuat1[0]) < 0.0) {
		dualQuat1 *= -1.0;
	}
	if (dot(dualQuat0[0], dualQuat2[0]) < 0.0) {
		dualQuat2 *= -1.0;
	}
	if (dot(dualQuat0[0], dualQuat3[0]) < 0.0) {
		dualQuat3 *= -1.0;
	}

	mat2x4 blendDualQuat = dualQuat0 * skinWeight.x;
	blendDualQuat += dualQuat1 * skinWeight.y;
	blendDualQuat += dualQuat2 * skinWeight.z;
	blendDualQuat += dualQuat3 * skinWeight.w;

	vec4 position;
	vec3 normal;

	// 对偶四元素还原Matrix进行计算
	if (bonesData.debugParam.x < 1.0)
	{
		mat4x4 boneMatrix = DualQuat2Matrix(blendDualQuat[0], blendDualQuat[1]);
		position = boneMatrix * vec4(inPosition.xyz, 1.0);
		normal   = mat3(boneMatrix) * inNormal;

		outColor = vec4(1.1, 1.0, 1.0, 1.0);
	}
	// 不还原，直接计算
	else
	{
		position.xyz = DualQuatTransformPosition(blendDualQuat, inPosition.xyz);
		position.w   = 1.0;
		normal = DualQuatTransformVector(blendDualQuat, inNormal);

		outColor = vec4(1.0, 1.1, 1.0, 1.0);
	}

	// 转换法线
	mat3 normalMatrix = transpose(inverse(mat3(uboMVP.modelMatrix)));
	normal = normalize(normalMatrix * normal);

	outUV     = inUV0;
	outNormal = normal;
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * position;
}