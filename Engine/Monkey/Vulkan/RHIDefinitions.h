#pragma once

#include "Core/PixelFormat.h"
#include "Common/Log.h"

#include <string>
#include <cstring>

enum ResLimit
{
    MAX_TEXTURE_MIP_COUNT = 14,
	MaxImmutableSamplers = 2,
	MaxVertexElementCount = 16,
	MaxVertexElementCount_NumBits = 4,
	MaxSimultaneousRenderTargets = 8,
	MaxSimultaneousRenderTargets_NumBits = 3,
	ShaderArrayElementAlignBytes = 16,
	MaxSimultaneousUAVs = 8
};

static_assert(MaxVertexElementCount <= (1 << MaxVertexElementCount_NumBits), "MaxVertexElementCount will not fit on MaxVertexElementCount_NumBits");
static_assert(MaxSimultaneousRenderTargets <= (1 << MaxSimultaneousRenderTargets_NumBits), "MaxSimultaneousRenderTargets will not fit on MaxSimultaneousRenderTargets_NumBits");

enum class ImageLayoutBarrier
{
    Undefined,
    TransferDest,
    ColorAttachment,
    DepthStencilAttachment,
    TransferSource,
    Present,
    PixelShaderRead,
    PixelDepthStencilRead,
    ComputeGeneralRW,
    PixelGeneralRW,
};

enum VertexAttribute
{
	VA_None = 0,
	VA_Position,
	VA_UV0,
	VA_UV1,
	VA_Normal,
	VA_Tangent,
	VA_Color,
	VA_SkinWeight,
	VA_SkinIndex,
	VA_SkinPack,
    VA_InstanceFloat1,
    VA_InstanceFloat2,
    VA_InstanceFloat3,
    VA_InstanceFloat4,
	VA_Custom0,
	VA_Custom1,
	VA_Custom2,
	VA_Custom3,
	VA_Count,
};

enum VertexElementType
{
    VET_None,
    VET_Float1,
    VET_Float2,
    VET_Float3,
    VET_Float4,
    VET_PackedNormal,
    VET_UByte4,
    VET_UByte4N,
    VET_Color,
    VET_Short2,
    VET_Short4,
    VET_Short2N,
    VET_Half2,
    VET_Half4,
    VET_Short4N,
    VET_UShort2,
    VET_UShort4,
    VET_UShort2N,
    VET_UShort4N,
    VET_URGB10A2N,
    VET_MAX,
    
    VET_NumBits = 5,
};

static_assert(VET_MAX <= (1 << VET_NumBits), "VET_MAX will not fit on VET_NumBits");

enum CubeFace
{
    CubeFace_PosX = 0,
    CubeFace_NegX,
    CubeFace_PosY,
    CubeFace_NegY,
    CubeFace_PosZ,
    CubeFace_NegZ,
    CubeFace_MAX
};

FORCE_INLINE VertexAttribute StringToVertexAttribute(const char* name)
{
	if (strcmp(name, "inPosition") == 0) {
		return VertexAttribute::VA_Position;
	}
	else if (strcmp(name, "inUV0") == 0) {
		return VertexAttribute::VA_UV0;
	}
	else if (strcmp(name, "inUV1") == 0) {
		return VertexAttribute::VA_UV1;
	}
	else if (strcmp(name, "inNormal") == 0) {
		return VertexAttribute::VA_Normal;
	}
	else if (strcmp(name, "inTangent") == 0) {
		return VertexAttribute::VA_Tangent;
	}
	else if (strcmp(name, "inColor") == 0) {
		return VertexAttribute::VA_Color;
	}
	else if (strcmp(name, "inSkinWeight") == 0) {
		return VertexAttribute::VA_SkinWeight;
	}
	else if (strcmp(name, "inSkinIndex") == 0) {
		return VertexAttribute::VA_SkinIndex;
	}
	else if (strcmp(name, "inSkinPack") == 0) {
		return VertexAttribute::VA_SkinPack;
	}
	else if (strcmp(name, "inCustom0") == 0) {
		return VertexAttribute::VA_Custom0;
	}
	else if (strcmp(name, "inCustom1") == 0) {
		return VertexAttribute::VA_Custom1;
	}
	else if (strcmp(name, "inCustom2") == 0) {
		return VertexAttribute::VA_Custom2;
	}
	else if (strcmp(name, "inCustom3") == 0) {
		return VertexAttribute::VA_Custom3;
	}
	
	return VertexAttribute::VA_None;
}
