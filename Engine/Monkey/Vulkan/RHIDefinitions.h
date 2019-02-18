#pragma once

#include "Core/PixelFormat.h"
#include "Common/Log.h"

enum ShaderFrequency
{
	SF_Vertex			= 0,
	SF_Hull				= 1,
	SF_Domain			= 2,
	SF_Pixel			= 3,
	SF_Geometry			= 4,
	SF_Compute			= 5,
	SF_NumFrequencies	= 6,
	SF_NumBits			= 3,
};

static_assert(SF_NumFrequencies <= (1 << SF_NumBits), "SF_NumFrequencies will not fit on SF_NumBits");

enum ShaderPlatform
{
    SP_VULKAN_SM5       = 0,
    SP_NumPlatforms     = 1,
    SP_NumBits          = 1,
};
static_assert(SP_NumPlatforms <= (1 << SP_NumBits), "SP_NumPlatforms will not fit on SP_NumBits");

enum RenderQueryType
{
    RQT_Undefined,
    RQT_Occlusion,
    RQT_AbsoluteTime,
};

enum
{
    MAX_TEXTURE_MIP_COUNT = 14
};

enum
{
    MaxImmutableSamplers = 2
};

enum
{
    MaxVertexElementCount = 16,
    MaxVertexElementCount_NumBits = 4,
};

static_assert(MaxVertexElementCount <= (1 << MaxVertexElementCount_NumBits), "MaxVertexElementCount will not fit on MaxVertexElementCount_NumBits");

enum
{
    ShaderArrayElementAlignBytes = 16
};

enum
{
    MaxSimultaneousRenderTargets = 8,
    MaxSimultaneousRenderTargets_NumBits = 3,
};

static_assert(MaxSimultaneousRenderTargets <= (1 << MaxSimultaneousRenderTargets_NumBits), "MaxSimultaneousRenderTargets will not fit on MaxSimultaneousRenderTargets_NumBits");

enum
{
    MaxSimultaneousUAVs = 8
};

enum class RHIZBuffer
{
    FarPlane   = 1,
    NearPlane  = 0,
    IsInverted = (int32)((int32)RHIZBuffer::FarPlane < (int32)RHIZBuffer::NearPlane),
};

namespace RHIFeatureLevel
{
    enum Type
    {
        SM5,
        Num
    };
};

namespace RHIShadingPath
{
    enum Type
    {
        Deferred,
        Forward,
        Num
    };
}

enum SamplerFilter
{
    SF_Point,
    SF_Bilinear,
    SF_Trilinear,
    SF_AnisotropicPoint,
    SF_AnisotropicLinear,
    
    SamplerFilter_Num,
    SamplerFilter_NumBits = 3,
};

static_assert(SamplerFilter_Num <= (1 << SamplerFilter_NumBits), "SamplerFilter_Num will not fit on SamplerFilter_NumBits");

enum SamplerAddressMode
{
    SAM_Wrap,
    SAM_Clamp,
    SAM_Mirror,
    SAM_Border,
    
    SamplerAddressMode_Num,
    SamplerAddressMode_NumBits = 2,
};
static_assert(SamplerAddressMode_Num <= (1 << SamplerAddressMode_NumBits), "SamplerAddressMode_Num will not fit on SamplerAddressMode_NumBits");

enum SamplerCompareFunction
{
    SCF_Never,
    SCF_Less
};

enum RasterizerFillMode
{
    RFM_Point,
    RFM_Wireframe,
    RFM_Solid,
    
    RasterizerFillMode_Num,
    RasterizerFillMode_NumBits = 2,
};

static_assert(RasterizerFillMode_Num <= (1 << RasterizerFillMode_NumBits), "RasterizerFillMode_Num will not fit on RasterizerFillMode_NumBits");

enum RasterizerCullMode
{
    RCM_None,
    RCM_CW,
    RCM_CCW,

    RasterizerCullMode_Num,
    RasterizerCullMode_NumBits = 2,
};

static_assert(RasterizerCullMode_Num <= (1 << RasterizerCullMode_NumBits), "RasterizerCullMode_Num will not fit on RasterizerCullMode_NumBits");

enum ColorWriteMask
{
    CW_RED   = 0x01,
    CW_GREEN = 0x02,
    CW_BLUE  = 0x04,
    CW_ALPHA = 0x08,
    
    CW_NONE  = 0,
    CW_RGB   = CW_RED | CW_GREEN | CW_BLUE,
    CW_RGBA  = CW_RED | CW_GREEN | CW_BLUE | CW_ALPHA,
    CW_RG    = CW_RED | CW_GREEN,
    CW_BA    = CW_BLUE | CW_ALPHA,
    
    ColorWriteMask_NumBits = 4,
};

enum CompareFunction
{
    CF_Less,
    CF_LessEqual,
    CF_Greater,
    CF_GreaterEqual,
    CF_Equal,
    CF_NotEqual,
    CF_Never,
    CF_Always,
    
    CompareFunction_Num,
    CompareFunction_NumBits = 3,
    
    CF_DepthNearOrEqual        = (((int32)RHIZBuffer::IsInverted != 0) ? CF_GreaterEqual : CF_LessEqual),
    CF_DepthNear               = (((int32)RHIZBuffer::IsInverted != 0) ? CF_Greater      : CF_Less),
    CF_DepthFartherOrEqual     = (((int32)RHIZBuffer::IsInverted != 0) ? CF_LessEqual    : CF_GreaterEqual),
    CF_DepthFarther            = (((int32)RHIZBuffer::IsInverted != 0) ? CF_Less         : CF_Greater),
};

static_assert(CompareFunction_Num <= (1 << CompareFunction_NumBits), "CompareFunction_Num will not fit on CompareFunction_NumBits");

enum StencilMask
{
    SM_Default,
    SM_255,
    SM_1,
    SM_2,
    SM_4,
    SM_8,
    SM_16,
    SM_32,
    SM_64,
    SM_128,
    SM_Count
};

enum StencilOp
{
    SO_Keep,
    SO_Zero,
    SO_Replace,
    SO_SaturatedIncrement,
    SO_SaturatedDecrement,
    SO_Invert,
    SO_Increment,
    SO_Decrement,

    StencilOp_Num,
    StencilOp_NumBits = 3,
};

static_assert(StencilOp_Num <= (1 << StencilOp_NumBits), "StencilOp_Num will not fit on StencilOp_NumBits");

enum BlendOperation
{
    BO_Add,
    BO_Subtract,
    BO_Min,
    BO_Max,
    BO_ReverseSubtract,

    BlendOperation_Num,
    BlendOperation_NumBits = 3,
};

static_assert(BlendOperation_Num <= (1 << BlendOperation_NumBits), "BlendOperation_Num will not fit on BlendOperation_NumBits");

enum BlendFactor
{
    BF_Zero,
    BF_One,
    BF_SourceColor,
    BF_InverseSourceColor,
    BF_SourceAlpha,
    BF_InverseSourceAlpha,
    BF_DestAlpha,
    BF_InverseDestAlpha,
    BF_DestColor,
    BF_InverseDestColor,
    BF_ConstantBlendFactor,
    BF_InverseConstantBlendFactor,

    BlendFactor_Num,
    BlendFactor_NumBits = 4,
};

static_assert(BlendFactor_Num <= (1 << BlendFactor_NumBits), "BlendFactor_Num will not fit on BlendFactor_NumBits");

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

enum UniformBufferUsage
{
    UniformBuffer_SingleDraw = 0,
    UniformBuffer_SingleFrame,
    UniformBuffer_MultiFrame,
};

enum UniformBufferBaseType
{
    UBMT_INVALID,
    UBMT_BOOL,
    UBMT_INT32,
    UBMT_UINT32,
    UBMT_FLOAT32,
    UBMT_STRUCT,
    UBMT_SRV,
    UBMT_UAV,
    UBMT_SAMPLER,
    UBMT_TEXTURE,
    
    EUniformBufferBaseType_Num,
    EUniformBufferBaseType_NumBits = 4,
};

static_assert(EUniformBufferBaseType_Num <= (1 << EUniformBufferBaseType_NumBits), "EUniformBufferBaseType_Num will not fit on EUniformBufferBaseType_NumBits");

struct RHIResourceTableEntry
{
public:
    static CONSTEXPR uint32 GetEndOfStreamToken()
    {
        return 0xffffffff;
    }
    
    static uint32 Create(uint16 uniformBufferIndex, uint16 resourceIndex, uint16 bindIndex)
    {
        return  ((uniformBufferIndex & RTD_Mask_UniformBufferIndex) << RTD_Shift_UniformBufferIndex) |
                ((resourceIndex & RTD_Mask_ResourceIndex) << RTD_Shift_ResourceIndex) |
                ((bindIndex & RTD_Mask_BindIndex) << RTD_Shift_BindIndex);
    }
    
    static inline uint16 GetUniformBufferIndex(uint32 data)
    {
        return (data >> RTD_Shift_UniformBufferIndex) & RTD_Mask_UniformBufferIndex;
    }
    
    static inline uint16 GetResourceIndex(uint32 data)
    {
        return (data >> RTD_Shift_ResourceIndex) & RTD_Mask_ResourceIndex;
    }

    static inline uint16 GetBindIndex(uint32 data)
    {
        return (data >> RTD_Shift_BindIndex) & RTD_Mask_BindIndex;
    }
	
private:
    
    enum ResourceTableDefinitions
    {
        RTD_NumBits_UniformBufferIndex      = 8,
        RTD_NumBits_ResourceIndex           = 16,
        RTD_NumBits_BindIndex               = 8,
        
        RTD_Mask_UniformBufferIndex         = (1 << RTD_NumBits_UniformBufferIndex) - 1,
        RTD_Mask_ResourceIndex              = (1 << RTD_NumBits_ResourceIndex) - 1,
        RTD_Mask_BindIndex                  = (1 << RTD_NumBits_BindIndex) - 1,

        RTD_Shift_BindIndex                 = 0,
        RTD_Shift_ResourceIndex             = RTD_Shift_BindIndex + RTD_NumBits_BindIndex,
        RTD_Shift_UniformBufferIndex        = RTD_Shift_ResourceIndex + RTD_NumBits_ResourceIndex,
    };
    
    static_assert(RTD_NumBits_UniformBufferIndex + RTD_NumBits_ResourceIndex + RTD_NumBits_BindIndex <= sizeof(uint32)* 8, "RTD_* values must fit in 32 bits");
};

enum ResourceLockMode
{
    RLM_ReadOnly,
    RLM_WriteOnly,
    RLM_Num
};

enum RangeCompressionMode
{
    RCM_UNorm,
    RCM_SNorm,
    RCM_MinMaxNorm,
    RCM_MinMax,
};

enum class PrimitiveTopologyType : uint8
{
    Triangle,
    Patch,
    Line,
    Point,
    Quad,
    
    Num,
    NumBits = 3,
};

static_assert((uint32)PrimitiveTopologyType::Num <= (1 << (uint32)PrimitiveTopologyType::NumBits), "PrimitiveTopologyType::Num will not fit on PrimitiveTopologyType::NumBits");

enum PrimitiveType
{
    PT_TriangleList,
    PT_TriangleStrip,
    PT_LineList,
    PT_QuadList,
    PT_PointList,
    PT_RectList,
    
    PT_1_ControlPointPatchList,
    PT_2_ControlPointPatchList,
    PT_3_ControlPointPatchList,
    PT_4_ControlPointPatchList,
    PT_5_ControlPointPatchList,
    PT_6_ControlPointPatchList,
    PT_7_ControlPointPatchList,
    PT_8_ControlPointPatchList,
    PT_9_ControlPointPatchList,
    PT_10_ControlPointPatchList,
    PT_11_ControlPointPatchList,
    PT_12_ControlPointPatchList,
    PT_13_ControlPointPatchList,
    PT_14_ControlPointPatchList,
    PT_15_ControlPointPatchList,
    PT_16_ControlPointPatchList,
    PT_17_ControlPointPatchList,
    PT_18_ControlPointPatchList,
    PT_19_ControlPointPatchList,
    PT_20_ControlPointPatchList,
    PT_21_ControlPointPatchList,
    PT_22_ControlPointPatchList,
    PT_23_ControlPointPatchList,
    PT_24_ControlPointPatchList,
    PT_25_ControlPointPatchList,
    PT_26_ControlPointPatchList,
    PT_27_ControlPointPatchList,
    PT_28_ControlPointPatchList,
    PT_29_ControlPointPatchList,
    PT_30_ControlPointPatchList,
    PT_31_ControlPointPatchList,
    PT_32_ControlPointPatchList,
    
    PT_Num,
    PT_NumBits = 6
};

static_assert(PT_Num <= (1 << 8), "PrimitiveType doesn't fit in a byte");
static_assert(PT_Num <= (1 << PT_NumBits), "PT_NumBits is too small");

enum BufferUsageFlags
{
    BUF_None              = 0x0000,
    BUF_Static            = 0x0001,
    BUF_Dynamic           = 0x0002,
    BUF_Volatile          = 0x0004,
    BUF_UnorderedAccess   = 0x0008,
    BUF_ByteAddressBuffer = 0x0020,
    BUF_UAVCounter        = 0x0040,
    BUF_StreamOutput      = 0x0080,
    BUF_DrawIndirect      = 0x0100,
    BUF_ShaderResource    = 0x0200,
    BUF_KeepCPUAccessible = 0x0400,
    BUF_ZeroStride        = 0x0800,
    BUF_FastVRAM          = 0x1000,
    BUF_Transient          = 0x2000,
    BUF_UINT8             = 0x4000,
    BUF_AnyDynamic = (BUF_Dynamic | BUF_Volatile),
};

enum RHIResourceType
{
    RRT_None,
    RRT_SamplerState,
    RRT_RasterizerState,
    RRT_DepthStencilState,
    RRT_BlendState,
    RRT_VertexDeclaration,
    RRT_VertexShader,
    RRT_HullShader,
    RRT_DomainShader,
    RRT_PixelShader,
    RRT_GeometryShader,
    RRT_ComputeShader,
    RRT_BoundShaderState,
    RRT_UniformBuffer,
    RRT_IndexBuffer,
    RRT_VertexBuffer,
    RRT_StructuredBuffer,
    RRT_Texture,
    RRT_Texture2D,
    RRT_Texture2DArray,
    RRT_Texture3D,
    RRT_TextureCube,
    RRT_TextureReference,
    RRT_RenderQuery,
    RRT_Viewport,
    RRT_UnorderedAccessView,
    RRT_ShaderResourceView,
    RRT_Num
};

enum ETextureCreateFlags
{
    TexCreate_None                    = 0,
    TexCreate_RenderTargetable        = 1 << 0,
    TexCreate_ResolveTargetable       = 1 << 1,
    TexCreate_DepthStencilTargetable  = 1 << 2,
    TexCreate_ShaderResource          = 1 << 3,
    TexCreate_SRGB                    = 1 << 4,
    TexCreate_CPUWritable             = 1 << 5,
    TexCreate_NoTiling                = 1 << 6,
    TexCreate_Dynamic                 = 1 << 8,
    TexCreate_InputAttachmentRead     = 1 << 9,
    TexCreate_DisableAutoDefrag       = 1 << 10,
    TexCreate_BiasNormalMap           = 1 << 11,
    TexCreate_GenerateMipCapable      = 1 << 12,
    TexCreate_FastVRAMPartialAlloc    = 1 << 13,
    TexCreate_UAV                     = 1 << 16,
    TexCreate_Presentable             = 1 << 17,
    TexCreate_CPUReadback             = 1 << 18,
    TexCreate_OfflineProcessed        = 1 << 19,
    TexCreate_FastVRAM                = 1 << 20,
    TexCreate_HideInVisualizeTexture  = 1 << 21,
    TexCreate_Virtual                 = 1 << 22,
    TexCreate_Shared                  = 1 << 23,
    TexCreate_NoFastClear             = 1 << 24,
    TexCreate_Streamable              = 1 << 25,
    TexCreate_NoFastClearFinalize     = 1 << 26,
    TexCreate_AFRManual               = 1 << 27,
    TexCreate_Transient               = 1 << 28,
    TexCreate_ReduceMemoryWithTilingMode     = 1 << 29,
    TexCreate_TargetArraySlicesIndependently = 1 << 30,
    TexCreate_DepthStencilResolveTarget      = 1 << 31,
};

enum AsyncComputePriority
{
    AsyncComputePriority_Default = 0,
    AsyncComputePriority_High,
};

enum TextureReallocationStatus
{
    TexRealloc_Succeeded = 0,
    TexRealloc_Failed,
    TexRealloc_InProgress,
};

enum class RenderTargetLoadAction : uint8
{
    RTLA_NoAction,
    RTLA_Load,
    RTLA_Clear,

    RTLA_Num,
    RTLA_NumBits = 2,
};

static_assert((uint32)RenderTargetLoadAction::RTLA_Num <= (1 << (uint32)RenderTargetLoadAction::RTLA_NumBits), "RenderTargetLoadAction::RTLA_Num will not fit on RenderTargetLoadAction::RTLA_NumBits");

enum class RenderTargetStoreAction : uint8
{
    RTSA_NoAction,
    RTSA_Store,
    RTSA_MultisampleResolve,
    RTSA_Num,
    RTSA_NumBits = 2,
};

static_assert((uint32)RenderTargetStoreAction::RTSA_Num <= (1 << (uint32)RenderTargetStoreAction::RTSA_NumBits), "RenderTargetStoreAction::RTSA_Num will not fit on RenderTargetStoreAction::RTSA_NumBits");

enum class SimpleRenderTargetMode
{
    SRTM_ExistingColorAndDepth,
    SRTM_UninitializedColorAndDepth,
    SRTM_UninitializedColorExistingDepth,
    SRTM_UninitializedColorClearDepth,
    SRTM_ClearColorExistingDepth,
    SRTM_ClearColorAndDepth,
    SRTM_ExistingContents_NoDepthStore,
    SRTM_ExistingColorAndClearDepth,
    SRTM_ExistingColorAndDepthAndClearStencil,
};

enum class ClearDepthStencil
{
    Depth,
    Stencil,
    DepthStencil,
};

enum class AsyncComputeBudget
{
    ELeast_0,
    EGfxHeavy_1,
    EBalanced_2,
    EComputeHeavy_3,
    EAll_4,
};

inline RHIFeatureLevel::Type GetMaxSupportedFeatureLevel(ShaderPlatform inShaderPlatform)
{
    return RHIFeatureLevel::SM5;
}

inline bool IsFeatureLevelSupported(ShaderPlatform inShaderPlatform, RHIFeatureLevel::Type inFeatureLevel)
{
    return inFeatureLevel <= GetMaxSupportedFeatureLevel(inShaderPlatform);
}

inline bool RHISupportsSeparateMSAAAndResolveTextures(const ShaderPlatform platform)
{
    return true;
}

inline bool RHISupportsComputeShaders(const ShaderPlatform platform)
{
    return IsFeatureLevelSupported(platform, RHIFeatureLevel::SM5);
}

inline bool RHISupportsGeometryShaders(const ShaderPlatform platform)
{
    return IsFeatureLevelSupported(platform, RHIFeatureLevel::SM5);
}

inline bool RHIHasTiledGPU(const ShaderPlatform platform)
{
    return false;
}

inline bool RHISupportsVertexShaderLayer(const ShaderPlatform platform)
{
    return IsFeatureLevelSupported(platform, RHIFeatureLevel::SM5);
}

inline bool RHISupportsDrawIndirect(const ShaderPlatform platform)
{
    return IsFeatureLevelSupported(platform, RHIFeatureLevel::SM5);
}

inline bool RHISupportsNativeShaderLibraries(const ShaderPlatform platform)
{
    return IsFeatureLevelSupported(platform, RHIFeatureLevel::SM5);
}

inline uint32 GetExpectedFeatureLevelMaxTextureSamplers(RHIFeatureLevel::Type featureLevel)
{
    return 16;
}

inline int32 GetFeatureLevelMaxNumberOfBones(RHIFeatureLevel::Type featureLevel)
{
    return 256;
}

inline bool IsUniformBufferResourceType(UniformBufferBaseType baseType)
{
    return baseType == UBMT_SRV || baseType == UBMT_UAV || baseType == UBMT_SAMPLER || baseType == UBMT_TEXTURE;
}

inline const char* GetShaderFrequencyString(ShaderFrequency frequency, bool bIncludePrefix = true)
{
    const char* result = "SF_NumFrequencies";
    
    switch (frequency)
    {
    case SF_Vertex:
        result = "SF_Vertex";
        break;
    case SF_Hull:
        result = "SF_Hull";
        break;
    case SF_Domain:
        result = "SF_Domain";
        break;
    case SF_Geometry:
        result = "SF_Geometry";
        break;
    case SF_Pixel:
        result = "SF_Pixel";
        break;
    case SF_Compute:
        result = "SF_Compute";
        break;
    default:
        MLOG("Unknown ShaderFrequency %d", (int32)frequency);
        break;
    }
    
    int32 index = bIncludePrefix ? 0 : 3;
    result += index;
    return result;
};
