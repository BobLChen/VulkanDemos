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

	vec4 animIndex;
} paramData;

layout (binding = 1) uniform sampler2D animMap;

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

mat2x4 ReadBoneAnim(int boneIndex, int startIndex)
{
	// 计算出当前帧动画的骨骼的索引
	int index = startIndex + boneIndex * 2;
	int animY = index / int(paramData.animIndex.x);
	int animX = index % int(paramData.animIndex.x);

	vec2 index0 = vec2(animX + 0.0f, animY) / paramData.animIndex.xy;
	vec2 index1 = vec2(animX + 1.0f, animY) / paramData.animIndex.xy;

	mat2x4 animData;
	animData[0] = texture(animMap, index0);
	animData[1] = texture(animMap, index1);

	return animData;
}

mat2x4 CalcDualQuat(ivec4 skinIndex, vec4 skinWeight, int startIndex)
{
	mat2x4 dualQuat0 = ReadBoneAnim(skinIndex.x, startIndex);
	mat2x4 dualQuat1 = ReadBoneAnim(skinIndex.y, startIndex);
	mat2x4 dualQuat2 = ReadBoneAnim(skinIndex.z, startIndex);
	mat2x4 dualQuat3 = ReadBoneAnim(skinIndex.w, startIndex);

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

	return blendDualQuat;
}

void main() 
{
	vec4 position;
	vec3 normal;

	if (paramData.animIndex.w > 0)
	{
		// skin info
		ivec4 skinIndex   = UnPackUInt32To4Byte(uint(inSkinPack.x));
		ivec2 skinWeight0 = UnPackUInt32To2Short(uint(inSkinPack.y));
		ivec2 skinWeight1 = UnPackUInt32To2Short(uint(inSkinPack.z));
		vec4  skinWeight  = vec4(skinWeight0 / 65535.0, skinWeight1 / 65535.0);

		mat2x4 dualQuat = CalcDualQuat(skinIndex, skinWeight, int(paramData.animIndex.z));

		position = vec4(DualQuatTransformPosition(dualQuat, inPosition.xyz), 1.0);
		normal   = DualQuatTransformVector(dualQuat, inNormal);
	}
	else
	{
		position = vec4(inPosition, 1.0);
		normal   = inNormal;
	}

	// 转换法线
	mat3 normalMatrix = transpose(inverse(mat3(paramData.modelMatrix)));
	normal = normalize(normalMatrix * normal);

	outUV     = inUV0;
	outNormal = normal;
	
	gl_Position = paramData.projectionMatrix * paramData.viewMatrix * paramData.modelMatrix * position;
}