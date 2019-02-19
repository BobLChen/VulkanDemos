#pragma once

#include "RHIDefinitions.h"
#include "PipelineFileCache.h"
#include "Common/Common.h"
#include "Common/Log.h"
#include "HAL/ThreadSafeCounter.h"
#include "Utils/StringUtils.h"
#include "Utils/SecureHash.h"
#include "Utils/Alignment.h"
#include "Math/Vector2.h"
#include "Math/Color.h"
#include <vector>
#include <string>
#include <memory>

struct RamAllocation
{
	RamAllocation(uint32 inAllocationStart = 0, uint32 inAllocationSize = 0)
		: allocationStart(inAllocationStart)
		, allocationSize(inAllocationSize)
	{

	}

	bool IsValid() const 
	{ 
		return allocationSize > 0;
	}

	uint32 allocationStart;
	uint32 allocationSize;
};

struct RHIResourceInfo
{
	RamAllocation ramAllocation;
};

enum class ClearBinding
{
	ENoneBound,
	EColorBound,
	EDepthStencilBound,
};

struct ClearValueBinding
{
public:
	static const ClearValueBinding None;
	static const ClearValueBinding Black;
	static const ClearValueBinding White;
	static const ClearValueBinding Transparent;
	static const ClearValueBinding DepthOne;
	static const ClearValueBinding DepthZero;
	static const ClearValueBinding DepthNear;
	static const ClearValueBinding DepthFar;
	static const ClearValueBinding Green;
	static const ClearValueBinding DefaultNormal8Bit;

	ClearBinding colorBinding;

public:
	struct DSVAlue
	{
		float depth;
		uint32 stencil;
	};

	union ClearValueType
	{
		float color[4];
		DSVAlue dsValue;
	} value;

	ClearValueBinding()
		: colorBinding(ClearBinding::EColorBound)
	{
		value.color[0] = 0.0f;
		value.color[1] = 0.0f;
		value.color[2] = 0.0f;
		value.color[3] = 0.0f;
	}

	ClearValueBinding(ClearBinding noBinding)
		: colorBinding(noBinding)
	{
		
	}

	explicit ClearValueBinding(const LinearColor& inClearColor)
		: colorBinding(ClearBinding::EColorBound)
	{
		value.color[0] = inClearColor.r;
		value.color[1] = inClearColor.g;
		value.color[2] = inClearColor.b;
		value.color[3] = inClearColor.a;
	}

	explicit ClearValueBinding(float depthClearValue, uint32 stencilClearValue = 0)
		: colorBinding(ClearBinding::EDepthStencilBound)
	{
		value.dsValue.depth = depthClearValue;
		value.dsValue.stencil = stencilClearValue;
	}

	LinearColor GetClearColor() const
	{
		return LinearColor(value.color[0], value.color[1], value.color[2], value.color[3]);
	}

	void GetDepthStencil(float& OutDepth, uint32& OutStencil) const
	{
		OutDepth = value.dsValue.depth;
		OutStencil = value.dsValue.stencil;
	}

	bool operator==(const ClearValueBinding& other) const
	{
		if (colorBinding == other.colorBinding)
		{
			if (colorBinding == ClearBinding::EColorBound)
			{
				return
					value.color[0] == other.value.color[0] &&
					value.color[1] == other.value.color[1] &&
					value.color[2] == other.value.color[2] &&
					value.color[3] == other.value.color[3];

			}
			if (colorBinding == ClearBinding::EDepthStencilBound)
			{
				return
					value.dsValue.depth == other.value.dsValue.depth &&
					value.dsValue.stencil == other.value.dsValue.stencil;
			}
			return true;
		}
		return false;
	}

};

class RHIResource
{
public:
	RHIResource()
		: m_Committed(true)
	{

	}

	virtual ~RHIResource()
	{
		if (m_NumRefs.GetValue() != 0)
		{
			MLOGE("RHIResource ref > 0");
		}
	}

	uint32 AddRef()
	{
		int32 newValue = m_NumRefs.Increment();
		return uint32(newValue);
	}

	uint32 Release()
	{
		int32 newValue = m_NumRefs.Decrement();
		if (newValue == 0)
		{
			delete this;
		}
		return uint32(newValue);
	}

	uint32 GetRefCount()
	{
		int32 curValue = m_NumRefs.GetValue();
		return uint32(curValue);
	}

	void SetCommitted(bool committed)
	{
		m_Committed = committed;
	}

	bool IsCommitted() const
	{
		return m_Committed;
	}

private:
	ThreadSafeCounter m_NumRefs;
	bool m_Committed;
};

//
// state
//

class RHISamplerState : public RHIResource
{
public:
	virtual bool IsImmutable() const 
	{ 
		return false;
	}
};

struct RasterizerStateInitializerRHI
{
	RasterizerFillMode fillMode;
	RasterizerCullMode cullMode;
	float depthBias;
	float slopeScaleDepthBias;
	bool allowMSAA;
	bool enableLineAA;
};

class RHIRasterizerState : public RHIResource
{
public:
	virtual bool GetInitializer(struct RasterizerStateInitializerRHI& init) 
	{ 
		return false;
	}
};

struct DepthStencilStateInitializerRHI
{
	bool enableDepthWrite;
	CompareFunction depthTest;

	bool enableFrontFaceStencil;
	CompareFunction frontFaceStencilTest;
	StencilOp frontFaceStencilFailStencilOp;
	StencilOp frontFaceDepthFailStencilOp;
	StencilOp frontFacePassStencilOp;

	bool enableBackFaceStencil;
	CompareFunction backFaceStencilTest;
	StencilOp backFaceStencilFailStencilOp;
	StencilOp backFaceDepthFailStencilOp;
	StencilOp backFacePassStencilOp;

	uint8 stencilReadMask;
	uint8 stencilWriteMask;

	DepthStencilStateInitializerRHI(
		bool inEnableDepthWrite = true,
		CompareFunction inDepthTest = CF_LessEqual,
		bool inEnableFrontFaceStencil = false,
		CompareFunction inFrontFaceStencilTest = CF_Always,
		StencilOp inFrontFaceStencilFailStencilOp = SO_Keep,
		StencilOp inFrontFaceDepthFailStencilOp = SO_Keep,
		StencilOp inFrontFacePassStencilOp = SO_Keep,
		bool inEnableBackFaceStencil = false,
		CompareFunction inBackFaceStencilTest = CF_Always,
		StencilOp inBackFaceStencilFailStencilOp = SO_Keep,
		StencilOp inBackFaceDepthFailStencilOp = SO_Keep,
		StencilOp inBackFacePassStencilOp = SO_Keep,
		uint8 inStencilReadMask = 0xFF,
		uint8 inStencilWriteMask = 0xFF
	)
		: enableDepthWrite(inEnableDepthWrite)
		, depthTest(inDepthTest)
		, enableFrontFaceStencil(inEnableFrontFaceStencil)
		, frontFaceStencilTest(inFrontFaceStencilTest)
		, frontFaceStencilFailStencilOp(inFrontFaceStencilFailStencilOp)
		, frontFaceDepthFailStencilOp(inFrontFaceDepthFailStencilOp)
		, frontFacePassStencilOp(inFrontFacePassStencilOp)
		, enableBackFaceStencil(inEnableBackFaceStencil)
		, backFaceStencilTest(inBackFaceStencilTest)
		, backFaceStencilFailStencilOp(inBackFaceStencilFailStencilOp)
		, backFaceDepthFailStencilOp(inBackFaceDepthFailStencilOp)
		, backFacePassStencilOp(inBackFacePassStencilOp)
		, stencilReadMask(inStencilReadMask)
		, stencilWriteMask(inStencilWriteMask)
	{

	}

	std::string ToString() const
	{
		return 
			StringUtils::Printf("<%u %u "
				, uint32(enableDepthWrite)
				, uint32(depthTest)
			)
			+ StringUtils::Printf("%u %u %u %u %u "
				, uint32(enableFrontFaceStencil)
				, uint32(frontFaceStencilTest)
				, uint32(frontFaceStencilFailStencilOp)
				, uint32(frontFaceDepthFailStencilOp)
				, uint32(frontFacePassStencilOp)
			)
			+ StringUtils::Printf("%u %u %u %u %u "
				, uint32(enableBackFaceStencil)
				, uint32(backFaceStencilTest)
				, uint32(backFaceStencilFailStencilOp)
				, uint32(backFaceDepthFailStencilOp)
				, uint32(backFacePassStencilOp)
			)
			+ StringUtils::Printf("%u %u>"
				, uint32(stencilReadMask)
				, uint32(stencilWriteMask)
			);
	}
};

class RHIDepthStencilState : public RHIResource
{
public:
	virtual bool GetInitializer(struct DepthStencilStateInitializerRHI& init)
	{ 
		return false;
	}
};

class BlendStateInitializerRHI
{
public:

	struct RenderTarget
	{
		enum
		{
			NUM_STRING_FIELDS = 7
		};

		BlendOperation	colorBlendOp;
		BlendFactor		colorSrcBlend;
		BlendFactor		colorDestBlend;
		BlendOperation	alphaBlendOp;
		BlendFactor		alphaSrcBlend;
		BlendFactor		alphaDestBlend;
		ColorWriteMask	colorWriteMask;

		RenderTarget(
			BlendOperation	inColorBlendOp		= BO_Add,
			BlendFactor		inColorSrcBlend		= BF_One,
			BlendFactor		inColorDestBlend	= BF_Zero,
			BlendOperation	inAlphaBlendOp		= BO_Add,
			BlendFactor		inAlphaSrcBlend		= BF_One,
			BlendFactor		inAlphaDestBlend	= BF_Zero,
			ColorWriteMask	inColorWriteMask	= CW_RGBA
		)
			: colorBlendOp(inColorBlendOp)
			, colorSrcBlend(inColorSrcBlend)
			, colorDestBlend(inColorDestBlend)
			, alphaBlendOp(inAlphaBlendOp)
			, alphaSrcBlend(inAlphaSrcBlend)
			, alphaDestBlend(inAlphaDestBlend)
			, colorWriteMask(inColorWriteMask)
		{

		}
	};

	BlendStateInitializerRHI()
		: m_RenderTargets(MaxSimultaneousRenderTargets)
	{

	}

	BlendStateInitializerRHI(const RenderTarget& renderTargetBlendState)
		: m_RenderTargets(MaxSimultaneousRenderTargets)
		, m_UseIndependentRenderTargetBlendStates(false)
	{
		m_RenderTargets[0] = renderTargetBlendState;
	}

	BlendStateInitializerRHI(const std::vector<RenderTarget>& inRenderTargetBlendStates)
		: m_RenderTargets(MaxSimultaneousRenderTargets)
		, m_UseIndependentRenderTargetBlendStates(inRenderTargetBlendStates.size() > 1)
	{
		for (uint32 i = 0; i < inRenderTargetBlendStates.size(); ++i)
		{
			m_RenderTargets[i] = inRenderTargetBlendStates[i];
		}
	}
	
	std::vector<RenderTarget> m_RenderTargets;
	bool m_UseIndependentRenderTargetBlendStates;
};

class RHIBlendState : public RHIResource
{
public:
	virtual bool GetInitializer(class BlendStateInitializerRHI& Init) 
	{ 
		return false;
	}
};

//
// Shader bindings
//


typedef std::vector<struct FVertexElement> VertexDeclarationElementList;
class RHIVertexDeclaration : public RHIResource
{
public:
	virtual bool GetInitializer(VertexDeclarationElementList& init) 
	{ 
		return false;
	}
};


class RHIBoundShaderState : public RHIResource 
{

};

//
// Shaders
//

class RHIShader : public RHIResource
{
public:
	void SetHash(SHAHash hash) 
	{ 
		m_Hash = hash;
	}

	SHAHash GetHash() const 
	{ 
		return m_Hash;
	}

	std::string shaderName;

private:
	SHAHash m_Hash;
};

class RHIVertexShader : public RHIShader
{

};

class RHIHullShader : public RHIShader 
{

};

class RHIDomainShader : public RHIShader 
{

};

class RHIPixelShader : public RHIShader 
{

};

class RHIGeometryShader : public RHIShader 
{

};

class RHIComputeShader : public RHIShader
{
public:
	RHIComputeShader() 
		: m_Stats(nullptr)
	{

	}

	inline void SetStats(struct PipelineStateStats* status) 
	{ 
		m_Stats = status;
	}

	void UpdateStats();

private:
	struct PipelineStateStats* m_Stats;
};

//
// Pipeline States
//

class RHIGraphicsPipelineState : public RHIResource 
{

};

class RHIComputePipelineState : public RHIResource 
{

};

//
// Buffers
//

struct RHIUniformBufferLayout
{
	enum Init
	{
		Zero
	};

	explicit RHIUniformBufferLayout(std::string name)
		: constantBufferSize(0)
		, m_Name(name)
		, m_Hash(0)
	{

	}

	explicit RHIUniformBufferLayout(Init) 
		: constantBufferSize(0)
		, m_Name("")
		, m_Hash(0)
	{

	}

	inline uint32 GetHash() const
	{
		return m_Hash;
	}

	void ComputeHash()
	{
		uint32 tmpHash = constantBufferSize << 16;

		for (int32 i = 0; i < resourceOffsets.size(); i++)
		{
			// Offset and therefore hash must be the same regardless of pointer size
			resourceOffsets[i] == Align(resourceOffsets[i], 8);
			tmpHash ^= resourceOffsets[i];
		}

		uint32 n = resources.size();
		while (n >= 4)
		{
			tmpHash ^= (resources[--n] << 0);
			tmpHash ^= (resources[--n] << 8);
			tmpHash ^= (resources[--n] << 16);
			tmpHash ^= (resources[--n] << 24);
		}
		while (n >= 2)
		{
			tmpHash ^= resources[--n] << 0;
			tmpHash ^= resources[--n] << 16;
		}
		while (n > 0)
		{
			tmpHash ^= resources[--n];
		}

		m_Hash = tmpHash;
	}

	void CopyFrom(const RHIUniformBufferLayout& source)
	{
		constantBufferSize = source.constantBufferSize;
		resourceOffsets = source.resourceOffsets;
		resources = source.resources;
		m_Name = source.m_Name;
		m_Hash = source.m_Hash;
	}

	const std::string GetName() const 
	{ 
		return m_Name;
	}

	uint32 constantBufferSize;
	std::vector<uint16> resourceOffsets;
	std::vector<uint8> resources;

private:
	std::string m_Name;
	uint32 m_Hash;
};

inline bool operator==(const RHIUniformBufferLayout& lhs, const RHIUniformBufferLayout& rhs)
{
	if (lhs.constantBufferSize != rhs.constantBufferSize) {
		return false;
	}
	if (lhs.resourceOffsets.size() != rhs.resourceOffsets.size()) {
		return false;
	}
	for (int i = 0; i < lhs.resourceOffsets.size(); ++i) {
		if (lhs.resourceOffsets[i] != rhs.resourceOffsets[i]) {
			return false;
		}
	}
	if (lhs.resources.size() != rhs.resources.size()) {
		return false;
	}
	for (int i = 0; i < lhs.resources.size(); ++i) {
		if (lhs.resources[i] != rhs.resources[i]) {
			return false;
		}
	}
	return true;
}

class RHIUniformBuffer : public RHIResource
{
public:

	RHIUniformBuffer(const RHIUniformBufferLayout& layout)
		: m_Layout(&layout)
		, m_LayoutConstantBufferSize(layout.constantBufferSize)
	{

	}

	uint32 GetSize() const
	{
		return m_LayoutConstantBufferSize;
	}

	const RHIUniformBufferLayout& GetLayout() const 
	{ 
		return *m_Layout;
	}

private:
	const RHIUniformBufferLayout* m_Layout;
	uint32 m_LayoutConstantBufferSize;
};

class RHIIndexBuffer : public RHIResource
{
public:

	RHIIndexBuffer(uint32 stride, uint32 size, uint32 usage)
		: m_Stride(stride)
		, m_Size(size)
		, m_Usage(usage)
	{

	}

	uint32 GetStride() const 
	{ 
		return m_Stride;
	}

	uint32 GetSize() const 
	{ 
		return m_Size;
	}

	uint32 GetUsage() const 
	{ 
		return m_Usage;
	}

private:
	uint32 m_Stride;
	uint32 m_Size;
	uint32 m_Usage;
};

class RHIVertexBuffer : public RHIResource
{
public:

	RHIVertexBuffer(uint32 size, uint32 usage)
		: m_Size(size)
		, m_Usage(usage)
	{

	}

	uint32 GetSize() const 
	{ 
		return m_Size;
	}

	uint32 GetUsage() const 
	{ 
		return m_Usage;
	}

private:
	uint32 m_Size;
	uint32 m_Usage;
};

class RHIStructuredBuffer : public RHIResource
{
public:

	RHIStructuredBuffer(uint32 stride, uint32 size, uint32 usage)
		: m_Stride(stride)
		, m_Size(size)
		, m_Usage(usage)
	{

	}

	uint32 GetStride() const 
	{ 
		return m_Stride;
	}

	uint32 GetSize() const 
	{ 
		return m_Size;
	}

	uint32 GetUsage() const 
	{ 
		return m_Usage;
	}

private:
	uint32 m_Stride;
	uint32 m_Size;
	uint32 m_Usage;
};


//
// Textures
//

class LastRenderTimeContainer
{
public:
	LastRenderTimeContainer() 
		: m_LastRenderTime(-FLT_MAX) 
	{

	}

	double GetLastRenderTime() const 
	{ 
		return m_LastRenderTime;
	}

	FORCEINLINE void SetLastRenderTime(double lastRenderTime)
	{
		if (m_LastRenderTime != lastRenderTime)
		{
			m_LastRenderTime = lastRenderTime;
		}
	}

private:
	double m_LastRenderTime;
};

class RHITexture : public RHIResource
{
public:

	RHITexture(uint32 numMips, uint32 numSamples, PixelFormat format, uint32 flags, LastRenderTimeContainer* lastRenderTime, const ClearValueBinding& clearValue)
		: m_ClearValue(clearValue)
		, m_NumMips(numMips)
		, m_NumSamples(numSamples)
		, m_Format(format)
		, m_Flags(flags)
		, m_LastRenderTime(lastRenderTime ? *lastRenderTime : m_DefaultLastRenderTime)
	{

	}

	virtual class RHITexture2D* GetTexture2D() 
	{ 
		return nullptr;
	}

	virtual class RHITexture2DArray* GetTexture2DArray() 
	{ 
		return nullptr;
	}

	virtual class RHITexture3D* GetTexture3D() 
	{ 
		return nullptr;
	}

	virtual class RHITextureCube* GetTextureCube() 
	{ 
		return nullptr;
	}

	virtual class RHITextureReference* GetTextureReference() 
	{ 
		return nullptr;
	}

	virtual void* GetNativeResource() const
	{
		return nullptr;
	}

	virtual void* GetNativeShaderResourceView() const
	{
		return nullptr;
	}

	virtual void* GetTextureBaseRHI()
	{
		return nullptr;
	}

	uint32 GetNumMips() const 
	{ 
		return m_NumMips;
	}

	PixelFormat GetFormat() const 
	{
		return m_Format;
	}

	uint32 GetFlags() const 
	{ 
		return m_Flags;
	}

	uint32 GetNumSamples() const 
	{ 
		return m_NumSamples;
	}

	bool IsMultisampled() const 
	{ 
		return m_NumSamples > 1;
	}

	FORCEINLINE void SetLastRenderTime(float lastRenderTime)
	{
		m_LastRenderTime.SetLastRenderTime(lastRenderTime);
	}

	LastRenderTimeContainer* GetLastRenderTimeContainer()
	{
		if (&m_LastRenderTime == &m_DefaultLastRenderTime)
		{
			return nullptr;
		}
		return &m_LastRenderTime;
	}

	void SetName(const std::string& name)
	{
		m_TextureName = name;
	}

	const std::string& GetName()
	{
		return m_TextureName;
	}

	bool HasClearValue() const
	{
		return m_ClearValue.colorBinding != ClearBinding::ENoneBound;
	}

	LinearColor GetClearColor() const
	{
		return m_ClearValue.GetClearColor();
	}

	void GetDepthStencilClearValue(float& outDepth, uint32& outStencil) const
	{
		return m_ClearValue.GetDepthStencil(outDepth, outStencil);
	}

	float GetDepthClearValue() const
	{
		float depth;
		uint32 stencil;
		m_ClearValue.GetDepthStencil(depth, stencil);
		return depth;
	}

	uint32 GetStencilClearValue() const
	{
		float depth;
		uint32 stencil;
		m_ClearValue.GetDepthStencil(depth, stencil);
		return stencil;
	}

	const ClearValueBinding GetClearBinding() const
	{
		return m_ClearValue;
	}

	virtual IntVector GetSizeXYZ() const = 0;

	RHIResourceInfo ResourceInfo;

private:
	ClearValueBinding m_ClearValue;
	uint32 m_NumMips;
	uint32 m_NumSamples;
	PixelFormat m_Format;
	uint32 m_Flags;
	LastRenderTimeContainer& m_LastRenderTime;
	LastRenderTimeContainer m_DefaultLastRenderTime;
	std::string m_TextureName;
};

class RHITexture2D : public RHITexture
{
public:

	RHITexture2D(uint32 inSizeX, uint32 inSizeY, uint32 inNumMips, uint32 inNumSamples, PixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
		: RHITexture(inNumMips, inNumSamples, inFormat, inFlags, nullptr, inClearValue)
		, m_SizeX(inSizeX)
		, m_SizeY(inSizeY)
	{

	}

	virtual RHITexture2D* GetTexture2D() 
	{ 
		return this;
	}

	uint32 GetSizeX() const 
	{ 
		return m_SizeX;
	}

	uint32 GetSizeY() const 
	{ 
		return m_SizeY;
	}

	inline IntPoint GetSizeXY() const
	{
		return IntPoint(m_SizeX, m_SizeY);
	}

	virtual IntVector GetSizeXYZ() const final override
	{
		return IntVector(m_SizeX, m_SizeY, 1);
	}

private:
	uint32 m_SizeX;
	uint32 m_SizeY;
};

class RHITexture2DArray : public RHITexture
{
public:

	RHITexture2DArray(uint32 inSizeX, uint32 inSizeY, uint32 inSizeZ, uint32 inNumMips, PixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
		: RHITexture(inNumMips, 1, inFormat, inFlags, nullptr, inClearValue)
		, m_SizeX(inSizeX)
		, m_SizeY(inSizeY)
		, m_SizeZ(inSizeZ)
	{

	}

	virtual RHITexture2DArray* GetTexture2DArray() 
	{ 
		return this;
	}

	uint32 GetSizeX() const 
	{ 
		return m_SizeX;
	}

	uint32 GetSizeY() const 
	{ 
		return m_SizeY;
	}

	uint32 GetSizeZ() const 
	{ 
		return m_SizeZ;
	}

	virtual IntVector GetSizeXYZ() const final override
	{
		return IntVector(m_SizeX, m_SizeY, m_SizeZ);
	}

private:
	uint32 m_SizeX;
	uint32 m_SizeY;
	uint32 m_SizeZ;
};

class RHITexture3D : public RHITexture
{
public:

	/** Initialization constructor. */
	RHITexture3D(uint32 inSizeX, uint32 inSizeY, uint32 inSizeZ, uint32 inNumMips, PixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
		: RHITexture(inNumMips, 1, inFormat, inFlags, NULL, inClearValue)
		, m_SizeX(inSizeX)
		, m_SizeY(inSizeY)
		, m_SizeZ(inSizeZ)
	{

	}

	virtual RHITexture3D* GetTexture3D() 
	{ 
		return this;
	}

	uint32 GetSizeX() const 
	{ 
		return m_SizeX;
	}

	uint32 GetSizeY() const 
	{ 
		return m_SizeY;
	}

	uint32 GetSizeZ() const 
	{ 
		return m_SizeZ;
	}

	virtual IntVector GetSizeXYZ() const final override
	{
		return IntVector(m_SizeX, m_SizeY, m_SizeZ);
	}

private:
	uint32 m_SizeX;
	uint32 m_SizeY;
	uint32 m_SizeZ;
};

class RHITextureCube : public RHITexture
{
public:
	RHITextureCube(uint32 inSize, uint32 inNumMips, PixelFormat inFormat, uint32 inFlags, const ClearValueBinding& inClearValue)
		: RHITexture(inNumMips, 1, inFormat, inFlags, nullptr, inClearValue)
		, m_Size(inSize)
	{

	}

	virtual RHITextureCube* GetTextureCube() 
	{ 
		return this;
	}

	uint32 GetSize() const 
	{ 
		return m_Size;
	}

	virtual IntVector GetSizeXYZ() const final override
	{
		return IntVector(m_Size, m_Size, 1);
	}

private:
	uint32 m_Size;
};

class RHITextureReference : public RHITexture
{
public:
	explicit RHITextureReference(LastRenderTimeContainer* inLastRenderTime)
		: RHITexture(0, 0, PF_Unknown, 0, inLastRenderTime, ClearValueBinding())
	{

	}

	virtual RHITextureReference* GetTextureReference() override 
	{ 
		return this;
	}

	inline RHITexture* GetReferencedTexture() const 
	{ 
		return m_ReferencedTexture;
	}

	void SetReferencedTexture(RHITexture* inTexture)
	{
		m_ReferencedTexture = inTexture;
	}

	virtual IntVector GetSizeXYZ() const final override
	{
		if (m_ReferencedTexture)
		{
			return m_ReferencedTexture->GetSizeXYZ();
		}
		return IntVector(0, 0, 0);
	}

private:
	RHITexture* m_ReferencedTexture;
};

class RHITextureReferenceNullImpl : public RHITextureReference
{
public:
	RHITextureReferenceNullImpl()
		: RHITextureReference(nullptr)
	{

	}

	void SetReferencedTexture(RHITexture* inTexture)
	{
		RHITextureReference::SetReferencedTexture(inTexture);
	}
};

//
// Misc
//

class RHIGPUFence : public RHIResource
{
public:
	RHIGPUFence(std::string name) 
		: m_FenceName(name)
	{

	}

	virtual ~RHIGPUFence() 
	{

	}

	virtual void Write();

	virtual bool Poll() const = 0;

private:
	std::string m_FenceName;
};

class GenericRHIGPUFence : public RHIGPUFence
{
public:
	GenericRHIGPUFence(std::string name);

	virtual void Write() final override;

	virtual bool Poll() const final override;

private:
	uint32 m_InsertedFrameNumber;
};

class RHIRenderQuery : public RHIResource 
{

};

class RHIComputeFence : public RHIResource
{
public:

	RHIComputeFence(std::string name)
		: m_Name(name)
		, m_WriteEnqueued(false)
	{

	}

	FORCEINLINE std::string GetName() const
	{
		return m_Name;
	}

	FORCEINLINE bool GetWriteEnqueued() const
	{
		return m_WriteEnqueued;
	}

	virtual void Reset()
	{
		m_WriteEnqueued = false;
	}

	virtual void WriteFence()
	{
		m_WriteEnqueued = true;
	}

private:
	std::string m_Name;
	bool m_WriteEnqueued;
};

class RHIViewport : public RHIResource
{
public:

	virtual void* GetNativeSwapChain() const 
	{ 
		return nullptr;
	}

	virtual void* GetNativeBackBufferTexture() const
	{ 
		return nullptr;
	}
	
	virtual void* GetNativeBackBufferRT() const 
	{ 
		return nullptr;
	}

	virtual void* GetNativeWindow(void** AddParam = nullptr) const 
	{ 
		return nullptr;
	}

	virtual void SetCustomPresent(class FRHICustomPresent*) 
	{

	}

	virtual class FRHICustomPresent* GetCustomPresent() const 
	{ 
		return nullptr;
	}

	virtual void Tick(float DeltaTime) 
	{

	}
};


class RHIUnorderedAccessView : public RHIResource 
{

};

class RHIShaderResourceView : public RHIResource 
{

};

typedef RHISamplerState*							SamplerStateRHIParamRef;
typedef std::shared_ptr<RHISamplerState>			SamplerStateRHIRef;

typedef RHIRasterizerState*							RasterizerStateRHIParamRef;
typedef std::shared_ptr<RHIRasterizerState>			RasterizerStateRHIRef;

typedef RHIDepthStencilState*						DepthStencilStateRHIParamRef;
typedef std::shared_ptr<RHIDepthStencilState>		DepthStencilStateRHIRef;

typedef RHIBlendState*								BlendStateRHIParamRef;
typedef std::shared_ptr<RHIBlendState>				BlendStateRHIRef;

typedef RHIVertexDeclaration*						VertexDeclarationRHIParamRef;
typedef std::shared_ptr<RHIVertexDeclaration>		VertexDeclarationRHIRef;

typedef RHIVertexShader*							VertexShaderRHIParamRef;
typedef std::shared_ptr<RHIVertexShader>			VertexShaderRHIRef;

typedef RHIHullShader*								HullShaderRHIParamRef;
typedef std::shared_ptr<RHIHullShader>				HullShaderRHIRef;

typedef RHIDomainShader*							DomainShaderRHIParamRef;
typedef std::shared_ptr<RHIDomainShader>			DomainShaderRHIRef;

typedef RHIPixelShader*								PixelShaderRHIParamRef;
typedef std::shared_ptr<RHIPixelShader>				PixelShaderRHIRef;

typedef RHIGeometryShader*							GeometryShaderRHIParamRef;
typedef std::shared_ptr<RHIGeometryShader>			GeometryShaderRHIRef;

typedef RHIComputeShader*							ComputeShaderRHIParamRef;
typedef std::shared_ptr<RHIComputeShader>			ComputeShaderRHIRef;

typedef RHIComputeFence*							ComputeFenceRHIParamRef;
typedef std::shared_ptr<RHIComputeFence>			ComputeFenceRHIRef;

typedef RHIBoundShaderState*						BoundShaderStateRHIParamRef;
typedef std::shared_ptr<RHIBoundShaderState>		BoundShaderStateRHIRef;

typedef RHIUniformBuffer*							UniformBufferRHIParamRef;
typedef std::shared_ptr<RHIUniformBuffer>			UniformBufferRHIRef;

typedef RHIIndexBuffer*								IndexBufferRHIParamRef;
typedef std::shared_ptr<RHIIndexBuffer>				IndexBufferRHIRef;

typedef RHIVertexBuffer*							VertexBufferRHIParamRef;
typedef std::shared_ptr<RHIVertexBuffer>			VertexBufferRHIRef;

typedef RHIStructuredBuffer*						StructuredBufferRHIParamRef;
typedef std::shared_ptr<RHIStructuredBuffer>		StructuredBufferRHIRef;

typedef RHITexture*									TextureRHIParamRef;
typedef std::shared_ptr<RHITexture>					TextureRHIRef;

typedef RHITexture2D*								Texture2DRHIParamRef;
typedef std::shared_ptr<RHITexture2D>				Texture2DRHIRef;

typedef RHITexture2DArray*							Texture2DArrayRHIParamRef;
typedef std::shared_ptr<RHITexture2DArray>			Texture2DArrayRHIRef;

typedef RHITexture3D*								Texture3DRHIParamRef;
typedef std::shared_ptr<RHITexture3D>				Texture3DRHIRef;

typedef RHITextureCube*								TextureCubeRHIParamRef;
typedef std::shared_ptr<RHITextureCube>				TextureCubeRHIRef;

typedef RHITextureReference*						TextureReferenceRHIParamRef;
typedef std::shared_ptr<RHITextureReference>		TextureReferenceRHIRef;

typedef RHIRenderQuery*								RenderQueryRHIParamRef;
typedef std::shared_ptr<RHIRenderQuery>				RenderQueryRHIRef;

typedef RHIGPUFence*								GPUFenceRHIParamRef;
typedef std::shared_ptr<RHIGPUFence>				GPUFenceRHIRef;

typedef RHIViewport*								ViewportRHIParamRef;
typedef std::shared_ptr<RHIViewport>				ViewportRHIRef;

typedef RHIUnorderedAccessView*						UnorderedAccessViewRHIParamRef;
typedef std::shared_ptr<RHIUnorderedAccessView>		UnorderedAccessViewRHIRef;

typedef RHIShaderResourceView*						ShaderResourceViewRHIParamRef;
typedef std::shared_ptr<RHIShaderResourceView>		ShaderResourceViewRHIRef;

typedef RHIGraphicsPipelineState*					GraphicsPipelineStateRHIParamRef;
typedef std::shared_ptr<RHIGraphicsPipelineState>	GraphicsPipelineStateRHIRef;

class RHIStagingBuffer : public RHIResource
{
public:
	RHIStagingBuffer(VertexBufferRHIParamRef InBuffer)
		: m_BackingBuffer(InBuffer)
	{

	}
	
	VertexBufferRHIParamRef GetBackingBuffer() const 
	{ 
		return m_BackingBuffer.get();
	}
protected:

	VertexBufferRHIRef m_BackingBuffer;
};

typedef RHIStagingBuffer*					StagingBufferRHIParamRef;
typedef std::shared_ptr<RHIStagingBuffer>	StagingBufferRHIRef;

class RHIRenderTargetView
{
public:
	TextureRHIParamRef texture;
	uint32 mipIndex;
	uint32 arraySliceIndex;
	RenderTargetLoadAction loadAction;
	RenderTargetStoreAction storeAction;

public:
	RHIRenderTargetView() 
		:texture(NULL)
		, mipIndex(0)
		, arraySliceIndex(-1)
		, loadAction(RenderTargetLoadAction::RTLA_NoAction)
		, storeAction(RenderTargetStoreAction::RTSA_NoAction)
	{
		
	}

	RHIRenderTargetView(const RHIRenderTargetView& other) 
		: texture(other.texture)
		, mipIndex(other.mipIndex)
		, arraySliceIndex(other.arraySliceIndex)
		, loadAction(other.loadAction)
		, storeAction(other.storeAction)
	{

	}

	explicit RHIRenderTargetView(TextureRHIParamRef inTexture, RenderTargetLoadAction inLoadAction) 
		: texture(inTexture)
		, mipIndex(0)
		, arraySliceIndex(-1)
		, loadAction(inLoadAction)
		, storeAction(RenderTargetStoreAction::RTSA_Store)
	{

	}

	explicit RHIRenderTargetView(TextureRHIParamRef inTexture, RenderTargetLoadAction inLoadAction, uint32 inMipIndex, uint32 inArraySliceIndex) 
		: texture(inTexture)
		, mipIndex(inMipIndex)
		, arraySliceIndex(inArraySliceIndex)
		, loadAction(inLoadAction)
		, storeAction(RenderTargetStoreAction::RTSA_Store)
	{

	}

	explicit RHIRenderTargetView(TextureRHIParamRef inTexture, uint32 inMipIndex, uint32 inArraySliceIndex, RenderTargetLoadAction inLoadAction, RenderTargetStoreAction inStoreAction) 
		: texture(inTexture)
		, mipIndex(inMipIndex)
		, arraySliceIndex(inArraySliceIndex)
		, loadAction(inLoadAction)
		, storeAction(inStoreAction)
	{

	}

	bool operator==(const RHIRenderTargetView& other) const
	{
		return
			texture == other.texture &&
			mipIndex == other.mipIndex &&
			arraySliceIndex == other.arraySliceIndex &&
			loadAction == other.loadAction &&
			storeAction == other.storeAction;
	}
};

class ExclusiveDepthStencil
{
public:
	enum Type
	{
		DepthNop		= 0x00,
		DepthRead		= 0x01,
		DepthWrite		= 0x02,
		DepthMask		= 0x0f,
		StencilNop		= 0x00,
		StencilRead		= 0x10,
		StencilWrite	= 0x20,
		StencilMask		= 0xf0,

		DepthNop_StencilNop		= DepthNop + StencilNop,
		DepthRead_StencilNop	= DepthRead + StencilNop,
		DepthWrite_StencilNop	= DepthWrite + StencilNop,
		DepthNop_StencilRead	= DepthNop + StencilRead,
		DepthRead_StencilRead	= DepthRead + StencilRead,
		DepthWrite_StencilRead	= DepthWrite + StencilRead,
		DepthNop_StencilWrite	= DepthNop + StencilWrite,
		DepthRead_StencilWrite	= DepthRead + StencilWrite,
		DepthWrite_StencilWrite = DepthWrite + StencilWrite,
	};

public:

	static const uint32 maxIndex = 4;

	ExclusiveDepthStencil(Type inValue = DepthNop_StencilNop)
		: m_Value(inValue)
	{

	}

	inline bool IsUsingDepthStencil() const
	{
		return m_Value != DepthNop_StencilNop;
	}

	inline bool IsUsingDepth() const
	{
		return (ExtractDepth() != DepthNop);
	}

	inline bool IsUsingStencil() const
	{
		return (ExtractStencil() != StencilNop);
	}

	inline bool IsDepthWrite() const
	{
		return ExtractDepth() == DepthWrite;
	}

	inline bool IsStencilWrite() const
	{
		return ExtractStencil() == StencilWrite;
	}

	inline bool IsAnyWrite() const
	{
		return IsDepthWrite() || IsStencilWrite();
	}

	inline void SetDepthWrite()
	{
		m_Value = (Type)(ExtractStencil() | DepthWrite);
	}

	inline void SetStencilWrite()
	{
		m_Value = (Type)(ExtractDepth() | StencilWrite);
	}

	inline void SetDepthStencilWrite(bool depth, bool stencil)
	{
		m_Value = DepthNop_StencilNop;

		if (depth)
		{
			SetDepthWrite();
		}
		if (stencil)
		{
			SetStencilWrite();
		}
	}

	bool operator==(const ExclusiveDepthStencil& rhs) const
	{
		return m_Value == rhs.m_Value;
	}

	bool operator != (const ExclusiveDepthStencil& rhs) const
	{
		return m_Value != rhs.m_Value;
	}

	inline bool IsValid(ExclusiveDepthStencil& current) const
	{
		Type depth = ExtractDepth();

		if (depth != DepthNop && depth != current.ExtractDepth())
		{
			return false;
		}

		Type stencil = ExtractStencil();

		if (stencil != StencilNop && stencil != current.ExtractStencil())
		{
			return false;
		}

		return true;
	}

	uint32 GetIndex() const
	{
		switch (m_Value)
		{
		case DepthWrite_StencilNop:
		case DepthNop_StencilWrite:
		case DepthWrite_StencilWrite:
		case DepthNop_StencilNop:
			return 0;

		case DepthRead_StencilNop:
		case DepthRead_StencilWrite:
			return 1;

		case DepthNop_StencilRead:
		case DepthWrite_StencilRead:
			return 2;

		case DepthRead_StencilRead:
			return 3;
		}
		return -1;
	}

private:

	inline Type ExtractDepth() const
	{
		return (Type)(m_Value & DepthMask);
	}

	inline Type ExtractStencil() const
	{
		return (Type)(m_Value & StencilMask);
	}

	Type m_Value;
};

class RHIDepthRenderTargetView
{

public:
	TextureRHIParamRef			texture;
	RenderTargetLoadAction		depthLoadAction;
	RenderTargetStoreAction		depthStoreAction;
	RenderTargetLoadAction		stencilLoadAction;

private:
	RenderTargetStoreAction		m_StencilStoreAction;
	ExclusiveDepthStencil		m_DepthStencilAccess;

public:

	explicit RHIDepthRenderTargetView() 
		: texture(nullptr)
		, depthLoadAction(RenderTargetLoadAction::RTLA_NoAction)
		, depthStoreAction(RenderTargetStoreAction::RTSA_NoAction)
		, stencilLoadAction(RenderTargetLoadAction::RTLA_NoAction)
		, m_StencilStoreAction(RenderTargetStoreAction::RTSA_NoAction)
		, m_DepthStencilAccess(ExclusiveDepthStencil::DepthNop_StencilNop)
	{

	}

	explicit RHIDepthRenderTargetView(TextureRHIParamRef inTexture, RenderTargetLoadAction inLoadAction, RenderTargetStoreAction inStoreAction) 
		: texture(inTexture)
		, depthLoadAction(inLoadAction)
		, depthStoreAction(inStoreAction)
		, stencilLoadAction(inLoadAction)
		, m_StencilStoreAction(inStoreAction)
		, m_DepthStencilAccess(ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{

	}

	explicit RHIDepthRenderTargetView(TextureRHIParamRef inTexture, RenderTargetLoadAction inLoadAction, RenderTargetStoreAction inStoreAction, ExclusiveDepthStencil InDepthStencilAccess) :
		texture(inTexture),
		depthLoadAction(inLoadAction),
		depthStoreAction(inStoreAction),
		stencilLoadAction(inLoadAction),
		m_StencilStoreAction(inStoreAction),
		m_DepthStencilAccess(InDepthStencilAccess)
	{

	}

	explicit RHIDepthRenderTargetView(TextureRHIParamRef inTexture, RenderTargetLoadAction inDepthLoadAction, RenderTargetStoreAction inDepthStoreAction, RenderTargetLoadAction inStencilLoadAction, RenderTargetStoreAction inStencilStoreAction) :
		texture(inTexture),
		depthLoadAction(inDepthLoadAction),
		depthStoreAction(inDepthStoreAction),
		stencilLoadAction(inStencilLoadAction),
		m_StencilStoreAction(inStencilStoreAction),
		m_DepthStencilAccess(ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{

	}

	explicit RHIDepthRenderTargetView(TextureRHIParamRef inTexture, RenderTargetLoadAction inDepthLoadAction, RenderTargetStoreAction inDepthStoreAction, RenderTargetLoadAction inStencilLoadAction, RenderTargetStoreAction inStencilStoreAction, ExclusiveDepthStencil inDepthStencilAccess) :
		texture(inTexture),
		depthLoadAction(inDepthLoadAction),
		depthStoreAction(inDepthStoreAction),
		stencilLoadAction(inStencilLoadAction),
		m_StencilStoreAction(inStencilStoreAction),
		m_DepthStencilAccess(inDepthStencilAccess)
	{

	}

	RenderTargetStoreAction GetStencilStoreAction() const
	{
		return m_StencilStoreAction;
	}

	ExclusiveDepthStencil GetDepthStencilAccess() const
	{
		return m_DepthStencilAccess;
	}

	bool operator==(const RHIDepthRenderTargetView& other) const
	{
		return
			texture == other.texture &&
			depthLoadAction == other.depthLoadAction &&
			depthStoreAction == other.depthStoreAction &&
			stencilLoadAction == other.stencilLoadAction &&
			m_StencilStoreAction == other.m_StencilStoreAction &&
			m_DepthStencilAccess == other.m_DepthStencilAccess;
	}
};

class RHISetRenderTargetsInfo
{
public:
	RHIRenderTargetView colorRenderTarget[MaxSimultaneousRenderTargets];
	int32 numColorRenderTargets;
	bool clearColor;

	RHIDepthRenderTargetView depthStencilRenderTarget;
	bool clearDepth;
	bool clearStencil;

	UnorderedAccessViewRHIRef unorderedAccessView[MaxSimultaneousUAVs];
	int32 numUAVs;

	RHISetRenderTargetsInfo() 
		: numColorRenderTargets(0)
		, clearColor(false)
		, clearDepth(false)
		, clearStencil(false)
		, numUAVs(0)
	{

	}

	RHISetRenderTargetsInfo(int32 inNumColorRenderTargets, const RHIRenderTargetView* inColorRenderTargets, const RHIDepthRenderTargetView& inDepthStencilRenderTarget) 
		: numColorRenderTargets(inNumColorRenderTargets)
		, clearColor(inNumColorRenderTargets > 0 && inColorRenderTargets[0].loadAction == RenderTargetLoadAction::RTLA_Clear)
		, depthStencilRenderTarget(inDepthStencilRenderTarget)
		, clearDepth(inDepthStencilRenderTarget.texture && inDepthStencilRenderTarget.depthLoadAction == RenderTargetLoadAction::RTLA_Clear)
		, clearStencil(inDepthStencilRenderTarget.texture && inDepthStencilRenderTarget.stencilLoadAction == RenderTargetLoadAction::RTLA_Clear)
		, numUAVs(0)
	{
		for (int32 index = 0; index < inNumColorRenderTargets; ++index)
		{
			colorRenderTarget[index] = inColorRenderTargets[index];
		}
	}

	void SetClearDepthStencil(bool inClearDepth, bool inClearStencil = false)
	{
		if (inClearDepth)
		{
			depthStencilRenderTarget.depthLoadAction = RenderTargetLoadAction::RTLA_Clear;
		}
		if (inClearStencil)
		{
			depthStencilRenderTarget.stencilLoadAction = RenderTargetLoadAction::RTLA_Clear;
		}
		clearDepth = inClearDepth;
		clearStencil = inClearStencil;
	}
};

struct BoundShaderStateInput
{
	VertexDeclarationRHIParamRef	vertexDeclarationRHI;
	VertexShaderRHIParamRef			vertexShaderRHI;
	HullShaderRHIParamRef			hullShaderRHI;
	DomainShaderRHIParamRef			domainShaderRHI;
	PixelShaderRHIParamRef			pixelShaderRHI;
	GeometryShaderRHIParamRef		geometryShaderRHI;

	FORCEINLINE BoundShaderStateInput()
		: vertexDeclarationRHI(nullptr)
		, vertexShaderRHI(nullptr)
		, hullShaderRHI(nullptr)
		, domainShaderRHI(nullptr)
		, pixelShaderRHI(nullptr)
		, geometryShaderRHI(nullptr)
	{

	}

	FORCEINLINE BoundShaderStateInput(
		VertexDeclarationRHIParamRef inVertexDeclarationRHI,
		VertexShaderRHIParamRef inVertexShaderRHI,
		HullShaderRHIParamRef inHullShaderRHI,
		DomainShaderRHIParamRef inDomainShaderRHI,
		PixelShaderRHIParamRef inPixelShaderRHI,
		GeometryShaderRHIParamRef inGeometryShaderRHI
	)
		: vertexDeclarationRHI(inVertexDeclarationRHI)
		, vertexShaderRHI(inVertexShaderRHI)
		, hullShaderRHI(inHullShaderRHI)
		, domainShaderRHI(inDomainShaderRHI)
		, pixelShaderRHI(inPixelShaderRHI)
		, geometryShaderRHI(inGeometryShaderRHI)
	{

	}
};

struct ImmutableSamplerState
{
	ImmutableSamplerState()
		: m_ImmutableSamplers(MaxImmutableSamplers)
	{

	}

	void Reset()
	{
		for (uint32 i = 0; i < MaxImmutableSamplers; ++i)
		{
			m_ImmutableSamplers[i] = nullptr;
		}
	}

	bool operator==(const std::vector<SamplerStateRHIParamRef>& rhs) const
	{
		if (m_ImmutableSamplers.size() != rhs.size()) {
			return false;
		}
		for (int i = 0; i < rhs.size(); ++i) {
			if (m_ImmutableSamplers[i] != rhs[i]) {
				return false;
			}
		}
		return true;
	}

	bool operator!=(const ImmutableSamplerState& rhs) const
	{
		if (m_ImmutableSamplers.size() != rhs.m_ImmutableSamplers.size()) {
			return true;
		}
		for (int i = 0; i < m_ImmutableSamplers.size(); ++i) {
			if (m_ImmutableSamplers[i] != rhs.m_ImmutableSamplers[i]) {
				return true;
			}
		}
		return false;
	}
	
	std::vector<SamplerStateRHIParamRef> m_ImmutableSamplers;
};

class GraphicsPipelineStateInitializer final
{
public:
	using RenderTargetFormats = std::vector<PixelFormat>;
	using RenderTargetFlags   = std::vector<uint32>;

	GraphicsPipelineStateInitializer()
		: blendState(nullptr)
		, rasterizerState(nullptr)
		, depthStencilState(nullptr)
		, primitiveType(PT_Num)
		, renderTargetsEnabled(0)
		, renderTargetFormats(PF_Unknown)
		, renderTargetFlags(0)
		, depthStencilTargetFormat(PF_Unknown)
		, depthStencilTargetFlag(0)
		, depthTargetLoadAction(RenderTargetLoadAction::RTLA_NoAction)
		, depthTargetStoreAction(RenderTargetStoreAction::RTSA_NoAction)
		, stencilTargetLoadAction(RenderTargetLoadAction::RTLA_NoAction)
		, stencilTargetStoreAction(RenderTargetStoreAction::RTSA_NoAction)
		, numSamples(0)
		, flags(0)
	{
		std::memset(this, 0, sizeof(GraphicsPipelineStateInitializer));
	}

	GraphicsPipelineStateInitializer(
		BoundShaderStateInput				inBoundShaderState,
		BlendStateRHIParamRef				inBlendState,
		RasterizerStateRHIParamRef			inRasterizerState,
		DepthStencilStateRHIParamRef		inDepthStencilState,
		ImmutableSamplerState				inImmutableSamplerState,
		PrimitiveType						inPrimitiveType,
		uint32								inRenderTargetsEnabled,
		const RenderTargetFormats&			inRenderTargetFormats,
		const RenderTargetFlags&			inRenderTargetFlags,
		PixelFormat							inDepthStencilTargetFormat,
		uint32								inDepthStencilTargetFlag,
		RenderTargetLoadAction				inDepthTargetLoadAction,
		RenderTargetStoreAction				inDepthTargetStoreAction,
		RenderTargetLoadAction				inStencilTargetLoadAction,
		RenderTargetStoreAction				inStencilTargetStoreAction,
		ExclusiveDepthStencil				inDepthStencilAccess,
		uint32								inNumSamples,
		uint16								inFlags
	)
		: boundShaderState(inBoundShaderState)
		, blendState(inBlendState)
		, rasterizerState(inRasterizerState)
		, depthStencilState(inDepthStencilState)
		, immutableSamplerState(inImmutableSamplerState)
		, primitiveType(inPrimitiveType)
		, renderTargetsEnabled(inRenderTargetsEnabled)
		, renderTargetFormats(inRenderTargetFormats)
		, renderTargetFlags(inRenderTargetFlags)
		, depthStencilTargetFormat(inDepthStencilTargetFormat)
		, depthStencilTargetFlag(inDepthStencilTargetFlag)
		, depthTargetLoadAction(inDepthTargetLoadAction)
		, depthTargetStoreAction(inDepthTargetStoreAction)
		, stencilTargetLoadAction(inStencilTargetLoadAction)
		, stencilTargetStoreAction(inStencilTargetStoreAction)
		, depthStencilAccess(inDepthStencilAccess)
		, numSamples(inNumSamples)
		, flags(inFlags)
	{
		std::memset(this, 0, sizeof(GraphicsPipelineStateInitializer));
		boundShaderState = inBoundShaderState;
		blendState = inBlendState;
		rasterizerState = inRasterizerState;
		depthStencilState = inDepthStencilState;
		primitiveType = inPrimitiveType;
		immutableSamplerState = inImmutableSamplerState;
		renderTargetsEnabled = inRenderTargetsEnabled;
		renderTargetFormats = inRenderTargetFormats;
		renderTargetFlags = inRenderTargetFlags;
		depthStencilTargetFormat = inDepthStencilTargetFormat;
		depthStencilTargetFlag = inDepthStencilTargetFlag;
		depthTargetLoadAction = inDepthTargetLoadAction;
		depthTargetStoreAction = inDepthTargetStoreAction;
		stencilTargetLoadAction = inStencilTargetLoadAction;
		stencilTargetStoreAction = inStencilTargetStoreAction;
		depthStencilAccess = inDepthStencilAccess;
		numSamples = inNumSamples;
		flags = inFlags;
	}

	bool operator==(const GraphicsPipelineStateInitializer& rhs) const
	{
		if (boundShaderState.vertexDeclarationRHI != rhs.boundShaderState.vertexDeclarationRHI ||
			boundShaderState.vertexShaderRHI != rhs.boundShaderState.vertexShaderRHI ||
			boundShaderState.pixelShaderRHI != rhs.boundShaderState.pixelShaderRHI ||
			boundShaderState.geometryShaderRHI != rhs.boundShaderState.geometryShaderRHI ||
			boundShaderState.domainShaderRHI != rhs.boundShaderState.domainShaderRHI ||
			boundShaderState.hullShaderRHI != rhs.boundShaderState.hullShaderRHI ||
			blendState != rhs.blendState ||
			rasterizerState != rhs.rasterizerState ||
			depthStencilState != rhs.depthStencilState ||
			immutableSamplerState != rhs.immutableSamplerState ||
			depthBounds != rhs.depthBounds ||
			primitiveType != rhs.primitiveType ||
			renderTargetsEnabled != rhs.renderTargetsEnabled ||
			renderTargetFormats != rhs.renderTargetFormats ||
			renderTargetFlags != rhs.renderTargetFlags ||
			depthStencilTargetFormat != rhs.depthStencilTargetFormat ||
			depthStencilTargetFlag != rhs.depthStencilTargetFlag ||
			depthTargetLoadAction != rhs.depthTargetLoadAction ||
			depthTargetStoreAction != rhs.depthTargetStoreAction ||
			stencilTargetLoadAction != rhs.stencilTargetLoadAction ||
			stencilTargetStoreAction != rhs.stencilTargetStoreAction ||
			depthStencilAccess != rhs.depthStencilAccess ||
			numSamples != rhs.numSamples)
		{
			return false;
		}

		return true;
	}

#define COMPARE_FIELD_BEGIN(Field) \
		if (Field != rhs.Field) \
		{ return Field COMPARE_OP rhs.Field; }

#define COMPARE_FIELD(Field) \
		else if (Field != rhs.Field) \
		{ return Field COMPARE_OP rhs.Field; }

#define COMPARE_FIELD_END \
		else { return false; }

	bool operator<(GraphicsPipelineStateInitializer& rhs) const
	{
#define COMPARE_OP <

		COMPARE_FIELD_BEGIN(boundShaderState.vertexDeclarationRHI)
		COMPARE_FIELD(boundShaderState.vertexShaderRHI)
		COMPARE_FIELD(boundShaderState.pixelShaderRHI)
		COMPARE_FIELD(boundShaderState.geometryShaderRHI)
		COMPARE_FIELD(boundShaderState.domainShaderRHI)
		COMPARE_FIELD(boundShaderState.hullShaderRHI)
		COMPARE_FIELD(blendState)
		COMPARE_FIELD(rasterizerState)
		COMPARE_FIELD(depthStencilState)
		COMPARE_FIELD(depthBounds)
		COMPARE_FIELD(primitiveType)
		COMPARE_FIELD_END;

#undef COMPARE_OP
	}

	bool operator>(GraphicsPipelineStateInitializer& rhs) const
	{
#define COMPARE_OP >

		COMPARE_FIELD_BEGIN(boundShaderState.vertexDeclarationRHI)
		COMPARE_FIELD(boundShaderState.vertexShaderRHI)
		COMPARE_FIELD(boundShaderState.pixelShaderRHI)
		COMPARE_FIELD(boundShaderState.geometryShaderRHI)
		COMPARE_FIELD(boundShaderState.domainShaderRHI)
		COMPARE_FIELD(boundShaderState.hullShaderRHI)
		COMPARE_FIELD(blendState)
		COMPARE_FIELD(rasterizerState)
		COMPARE_FIELD(depthStencilState)
		COMPARE_FIELD(depthBounds)
		COMPARE_FIELD(primitiveType)
		COMPARE_FIELD_END;

#undef COMPARE_OP
	}

#undef COMPARE_FIELD_BEGIN
#undef COMPARE_FIELD
#undef COMPARE_FIELD_END

	uint32 ComputeNumValidRenderTargets() const
	{
		if (renderTargetsEnabled > 0)
		{
			int32 lastValidTarget = -1;
			for (int32 i = (int32)renderTargetsEnabled - 1; i >= 0; i--)
			{
				if (renderTargetFormats[i] != PF_Unknown)
				{
					lastValidTarget = i;
					break;
				}
			}
			return uint32(lastValidTarget + 1);
		}
		return renderTargetsEnabled;
	}

	uint16							flags;
	BoundShaderStateInput			boundShaderState;
	BlendStateRHIParamRef			blendState;
	RasterizerStateRHIParamRef		rasterizerState;
	DepthStencilStateRHIParamRef	depthStencilState;
	ImmutableSamplerState			immutableSamplerState;
	bool							depthBounds = false;
	PrimitiveType					primitiveType;
	uint32							renderTargetsEnabled;
	RenderTargetFormats				renderTargetFormats;
	RenderTargetFlags				renderTargetFlags;
	PixelFormat						depthStencilTargetFormat;
	uint32							depthStencilTargetFlag;
	RenderTargetLoadAction			depthTargetLoadAction;
	RenderTargetStoreAction			depthTargetStoreAction;
	RenderTargetLoadAction			stencilTargetLoadAction;
	RenderTargetStoreAction			stencilTargetStoreAction;
	ExclusiveDepthStencil			depthStencilAccess;
	uint16							numSamples;
};

class RHIGraphicsPipelineStateFallBack : public RHIGraphicsPipelineState
{
public:
	RHIGraphicsPipelineStateFallBack() {}

	RHIGraphicsPipelineStateFallBack(const GraphicsPipelineStateInitializer& Init)
		: initializer(Init)
	{

	}

	GraphicsPipelineStateInitializer initializer;
};

class RHIComputePipelineStateFallback : public RHIComputePipelineState
{
public:
	RHIComputePipelineStateFallback(RHIComputeShader* InComputeShader)
		: m_ComputeShader(InComputeShader)
	{

	}

	RHIComputeShader* GetComputeShader()
	{
		return m_ComputeShader;
	}

protected:
	RHIComputeShader* m_ComputeShader;
};

class RHIShaderLibrary : public RHIResource
{
public:
	struct ShaderLibraryEntry
	{
		ShaderLibraryEntry() 
			: m_Frequency(SF_NumFrequencies)
			, m_Platform(SP_NumPlatforms)
		{

		}

		bool IsValid() const 
		{ 
			return (m_Frequency < SF_NumFrequencies) && (m_Frequency < SP_NumPlatforms);
		}

		SHAHash Hash;
		ShaderFrequency m_Frequency;
		ShaderPlatform m_Platform;
	};

	class ShaderLibraryIterator : public RHIResource
	{
	public:
		ShaderLibraryIterator(RHIShaderLibrary* shaderLibrary) 
			: m_ShaderLibrarySource(shaderLibrary)
		{

		}

		virtual ~ShaderLibraryIterator() 
		{

		}

		virtual bool IsValid() const = 0;

		virtual ShaderLibraryEntry operator*()	const = 0;

		virtual ShaderLibraryIterator& operator++() = 0;

		RHIShaderLibrary* GetLibrary() const 
		{ 
			return m_ShaderLibrarySource;
		};

	protected:
		RHIShaderLibrary* m_ShaderLibrarySource;
	};

	RHIShaderLibrary(ShaderPlatform inPlatform, std::string const& inName) 
		: m_Platform(inPlatform)
		, m_LibraryName(inName)
	{

	}

	virtual ~RHIShaderLibrary()
	{

	}

	FORCEINLINE ShaderPlatform GetPlatform(void) const 
	{ 
		return m_Platform;
	}

	FORCEINLINE std::string GetName(void) const 
	{ 
		return m_LibraryName;
	}

	virtual bool IsNativeLibrary() const = 0;

	virtual ShaderLibraryIterator* CreateIterator(void) = 0;

	virtual bool RequestEntry(const SHAHash& hash, std::vector<uint8>& outRaw)
	{
		return false;
	}

	virtual bool ContainsEntry(const SHAHash& Hash) = 0;

	virtual uint32 GetShaderCount(void) const = 0;

protected:
	ShaderPlatform m_Platform;
	std::string m_LibraryName;
};

typedef RHIShaderLibrary*					RHIShaderLibraryParamRef;
typedef std::shared_ptr<RHIShaderLibrary>	RHIShaderLibraryRef;

class RHIPipelineBinaryLibrary : public RHIResource
{
public:
	RHIPipelineBinaryLibrary(ShaderPlatform inPlatform, std::string const& filePath) 
		: m_Platform(inPlatform)
	{

	}

	virtual ~RHIPipelineBinaryLibrary() 
	{

	}

	FORCEINLINE ShaderPlatform GetPlatform(void) const 
	{ 
		return m_Platform;
	}

protected:
	ShaderPlatform m_Platform;
};

typedef RHIPipelineBinaryLibrary*					RHIPipelineBinaryLibraryParamRef;
typedef std::shared_ptr<RHIPipelineBinaryLibrary>	RHIPipelineBinaryLibraryRef;

enum class RenderTargetActions : uint8
{
	LoadOpMask = 2,
#define RTACTION_MAKE_MASK(Load, Store) (((uint8)RenderTargetLoadAction::RTLA_Load << (uint8)LoadOpMask) | (uint8)RenderTargetStoreAction::RTSA_Store)
	DontLoad_DontStore = RTACTION_MAKE_MASK(ENoAction, ENoAction),
	DontLoad_Store = RTACTION_MAKE_MASK(ENoAction, EStore),
	Clear_Store = RTACTION_MAKE_MASK(EClear, EStore),
	Load_Store = RTACTION_MAKE_MASK(ELoad, EStore),
	Clear_DontStore = RTACTION_MAKE_MASK(EClear, ENoAction),
	Load_DontStore = RTACTION_MAKE_MASK(ELoad, ENoAction),
	Clear_Resolve = RTACTION_MAKE_MASK(EClear, EMultisampleResolve),
	Load_Resolve = RTACTION_MAKE_MASK(ELoad, EMultisampleResolve),
#undef RTACTION_MAKE_MASK
};

inline RenderTargetActions MakeRenderTargetActions(RenderTargetLoadAction Load, RenderTargetStoreAction Store)
{
	return (RenderTargetActions)(((uint8)Load << (uint8)RenderTargetActions::LoadOpMask) | (uint8)Store);
}

inline RenderTargetLoadAction GetLoadAction(RenderTargetActions Action)
{
	return (RenderTargetLoadAction)((uint8)Action >> (uint8)RenderTargetActions::LoadOpMask);
}

inline RenderTargetStoreAction GetStoreAction(RenderTargetActions Action)
{
	return (RenderTargetStoreAction)((uint8)Action & ((1 << (uint8)RenderTargetActions::LoadOpMask) - 1));
}

enum class DepthStencilTargetActions : uint8
{
	DepthMask = 4,

#define RTACTION_MAKE_MASK(Depth, Stencil) (((uint8)RenderTargetActions::Depth << (uint8)DepthMask) | (uint8)RenderTargetActions::Stencil)

	DontLoad_DontStore = RTACTION_MAKE_MASK(DontLoad_DontStore, DontLoad_DontStore),
	DontLoad_StoreDepthStencil = RTACTION_MAKE_MASK(DontLoad_Store, DontLoad_Store),
	DontLoad_StoreStencilNotDepth = RTACTION_MAKE_MASK(DontLoad_DontStore, DontLoad_Store),
	ClearDepthStencil_StoreDepthStencil = RTACTION_MAKE_MASK(Clear_Store, Clear_Store),
	LoadDepthStencil_StoreDepthStencil = RTACTION_MAKE_MASK(Load_Store, Load_Store),
	LoadDepthNotStencil_DontStore = RTACTION_MAKE_MASK(Load_DontStore, DontLoad_DontStore),
	LoadDepthStencil_StoreStencilNotDepth = RTACTION_MAKE_MASK(Load_DontStore, Load_Store),

	ClearDepthStencil_DontStoreDepthStencil = RTACTION_MAKE_MASK(Clear_DontStore, Clear_DontStore),
	LoadDepthStencil_DontStoreDepthStencil = RTACTION_MAKE_MASK(Load_DontStore, Load_DontStore),
	ClearDepthStencil_StoreDepthNotStencil = RTACTION_MAKE_MASK(Clear_Store, Clear_DontStore),
	ClearDepthStencil_StoreStencilNotDepth = RTACTION_MAKE_MASK(Clear_DontStore, Clear_Store),
	ClearDepthStencil_ResolveDepthNotStencil = RTACTION_MAKE_MASK(Clear_Resolve, Clear_DontStore),
	ClearDepthStencil_ResolveStencilNotDepth = RTACTION_MAKE_MASK(Clear_DontStore, Clear_Resolve),

	ClearStencilDontLoadDepth_StoreStencilNotDepth = RTACTION_MAKE_MASK(DontLoad_DontStore, Clear_Store),

#undef RTACTION_MAKE_MASK
};

inline constexpr DepthStencilTargetActions MakeDepthStencilTargetActions(const RenderTargetActions Depth, const RenderTargetActions Stencil)
{
	return (DepthStencilTargetActions)(((uint8)Depth << (uint8)DepthStencilTargetActions::DepthMask) | (uint8)Stencil);
}

inline RenderTargetActions GetDepthActions(DepthStencilTargetActions Action)
{
	return (RenderTargetActions)((uint8)Action >> (uint8)DepthStencilTargetActions::DepthMask);
}

inline RenderTargetActions GetStencilActions(DepthStencilTargetActions Action)
{
	return (RenderTargetActions)((uint8)Action & ((1 << (uint8)DepthStencilTargetActions::DepthMask) - 1));
}

struct RHIRenderPassInfo
{
	struct ColorEntry
	{
		RHITexture* renderTarget;
		RHITexture* resolveTarget;
		int32 arraySlice;
		uint8 mipIndex;
		RenderTargetActions action;
	};
	ColorEntry colorRenderTargets[MaxSimultaneousRenderTargets];

	struct DepthStencilEntry
	{
		RHITexture* depthStencilTarget;
		RHITexture* resolveTarget;
		DepthStencilTargetActions action;
		ExclusiveDepthStencil exclusiveDepthStencil;
	};
	DepthStencilEntry depthStencilRenderTarget;

	uint32 numOcclusionQueries = 0;
	bool occlusionQueries = false;bool generatingMips = false;

	explicit RHIRenderPassInfo(RHITexture* colorRT, RenderTargetActions colorAction, RHITexture* resolveRT = nullptr, uint32 inMipIndex = 0, int32 inArraySlice = -1)
	{
		colorRenderTargets[0].renderTarget = colorRT;
		colorRenderTargets[0].resolveTarget = resolveRT;
		colorRenderTargets[0].arraySlice = inArraySlice;
		colorRenderTargets[0].mipIndex = inMipIndex;
		colorRenderTargets[0].action = colorAction;
		depthStencilRenderTarget.depthStencilTarget = nullptr;
		depthStencilRenderTarget.action = DepthStencilTargetActions::DontLoad_DontStore;
		depthStencilRenderTarget.exclusiveDepthStencil = ExclusiveDepthStencil::DepthNop_StencilNop;
		m_IsMSAA = colorRT->GetNumSamples() > 1;
		std::memset(&colorRenderTargets[1], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
	}

	explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], RenderTargetActions colorAction)
	{
		for (int32 index = 0; index < numColorRTs; ++index)
		{
			colorRenderTargets[index].renderTarget = colorRTs[index];
			colorRenderTargets[index].resolveTarget = nullptr;
			colorRenderTargets[index].arraySlice = -1;
			colorRenderTargets[index].mipIndex = 0;
			colorRenderTargets[index].action = colorAction;
		}

		depthStencilRenderTarget.depthStencilTarget = nullptr;
		depthStencilRenderTarget.action = DepthStencilTargetActions::DontLoad_DontStore;
		depthStencilRenderTarget.exclusiveDepthStencil = ExclusiveDepthStencil::DepthNop_StencilNop;
		
		if (numColorRTs < MaxSimultaneousRenderTargets)
		{
			std::memset(&colorRenderTargets[numColorRTs], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
		}
	}

	explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], RenderTargetActions colorAction, RHITexture* resolveTargets[])
	{
		for (int32 index = 0; index < numColorRTs; ++index)
		{
			colorRenderTargets[index].renderTarget = colorRTs[index];
			colorRenderTargets[index].resolveTarget = resolveTargets[index];
			colorRenderTargets[index].arraySlice = -1;
			colorRenderTargets[index].mipIndex = 0;
			colorRenderTargets[index].action = colorAction;
		}

		depthStencilRenderTarget.depthStencilTarget = nullptr;
		depthStencilRenderTarget.action = DepthStencilTargetActions::DontLoad_DontStore;
		depthStencilRenderTarget.exclusiveDepthStencil = ExclusiveDepthStencil::DepthNop_StencilNop;
		
		if (numColorRTs < MaxSimultaneousRenderTargets)
		{
			std::memset(&colorRenderTargets[numColorRTs], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
		}
	}

	explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], RenderTargetActions colorAction, RHITexture* depthRT, DepthStencilTargetActions depthActions, ExclusiveDepthStencil inEDS = ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{
		for (int32 index = 0; index < numColorRTs; ++index)
		{
			colorRenderTargets[index].renderTarget = colorRTs[index];
			colorRenderTargets[index].resolveTarget = nullptr;
			colorRenderTargets[index].arraySlice = -1;
			colorRenderTargets[index].mipIndex = 0;
			colorRenderTargets[index].action = colorAction;
		}

		depthStencilRenderTarget.depthStencilTarget = depthRT;
		depthStencilRenderTarget.resolveTarget = nullptr;
		depthStencilRenderTarget.action = depthActions;
		depthStencilRenderTarget.exclusiveDepthStencil = inEDS;
		m_IsMSAA = depthRT->GetNumSamples() > 1;

		if (numColorRTs < MaxSimultaneousRenderTargets)
		{
			std::memset(&colorRenderTargets[numColorRTs], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
		}
	}

	explicit RHIRenderPassInfo(int32 numColorRTs, RHITexture* colorRTs[], RenderTargetActions colorAction, RHITexture* ResolveRTs[], RHITexture* depthRT, DepthStencilTargetActions depthActions, RHITexture* ResolveDepthRT, ExclusiveDepthStencil inEDS = ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{
		for (int32 index = 0; index < numColorRTs; ++index)
		{
			colorRenderTargets[index].renderTarget = colorRTs[index];
			colorRenderTargets[index].resolveTarget = ResolveRTs[index];
			colorRenderTargets[index].arraySlice = -1;
			colorRenderTargets[index].mipIndex = 0;
			colorRenderTargets[index].action = colorAction;
		}

		depthStencilRenderTarget.depthStencilTarget = depthRT;
		depthStencilRenderTarget.resolveTarget = ResolveDepthRT;
		depthStencilRenderTarget.action = depthActions;
		depthStencilRenderTarget.exclusiveDepthStencil = inEDS;
		m_IsMSAA = depthRT->GetNumSamples() > 1;

		if (numColorRTs < MaxSimultaneousRenderTargets)
		{
			std::memset(&colorRenderTargets[numColorRTs], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRTs));
		}
	}

	explicit RHIRenderPassInfo(RHITexture* depthRT, DepthStencilTargetActions depthActions, RHITexture* ResolveDepthRT = nullptr, ExclusiveDepthStencil inEDS = ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{
		depthStencilRenderTarget.depthStencilTarget = depthRT;
		depthStencilRenderTarget.resolveTarget = ResolveDepthRT;
		depthStencilRenderTarget.action = depthActions;
		depthStencilRenderTarget.exclusiveDepthStencil = inEDS;
		m_IsMSAA = depthRT->GetNumSamples() > 1;
		std::memset(colorRenderTargets, 0, sizeof(ColorEntry) * MaxSimultaneousRenderTargets);
	}

	explicit RHIRenderPassInfo(RHITexture* depthRT, uint32 InNumOcclusionQueries, DepthStencilTargetActions depthActions, RHITexture* resolveDepthRT = nullptr, ExclusiveDepthStencil inEDS = ExclusiveDepthStencil::DepthWrite_StencilWrite)
		: numOcclusionQueries(InNumOcclusionQueries)
		, occlusionQueries(true)
	{
		depthStencilRenderTarget.depthStencilTarget = depthRT;
		depthStencilRenderTarget.resolveTarget = resolveDepthRT;
		depthStencilRenderTarget.action = depthActions;
		depthStencilRenderTarget.exclusiveDepthStencil = inEDS;
		m_IsMSAA = depthRT->GetNumSamples() > 1;
		std::memset(colorRenderTargets, 0, sizeof(ColorEntry) * MaxSimultaneousRenderTargets);
	}

	explicit RHIRenderPassInfo(RHITexture* colorRT, RenderTargetActions colorAction, RHITexture* depthRT, DepthStencilTargetActions depthActions, ExclusiveDepthStencil inEDS = ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{
		colorRenderTargets[0].renderTarget = colorRT;
		colorRenderTargets[0].resolveTarget = nullptr;
		colorRenderTargets[0].arraySlice = -1;
		colorRenderTargets[0].mipIndex = 0;
		colorRenderTargets[0].action = colorAction;
		m_IsMSAA = colorRT->GetNumSamples() > 1;
		depthStencilRenderTarget.depthStencilTarget = depthRT;
		depthStencilRenderTarget.resolveTarget = nullptr;
		depthStencilRenderTarget.action = depthActions;
		depthStencilRenderTarget.exclusiveDepthStencil = inEDS;
		std::memset(&colorRenderTargets[1], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
	}

	explicit RHIRenderPassInfo(RHITexture* colorRT, RenderTargetActions colorAction, RHITexture* resolveColorRT,
		RHITexture* depthRT, DepthStencilTargetActions depthActions, RHITexture* ResolveDepthRT, ExclusiveDepthStencil inEDS = ExclusiveDepthStencil::DepthWrite_StencilWrite)
	{
		colorRenderTargets[0].renderTarget = colorRT;
		colorRenderTargets[0].resolveTarget = resolveColorRT;
		colorRenderTargets[0].arraySlice = -1;
		colorRenderTargets[0].mipIndex = 0;
		colorRenderTargets[0].action = colorAction;
		m_IsMSAA = colorRT->GetNumSamples() > 1;
		depthStencilRenderTarget.depthStencilTarget = depthRT;
		depthStencilRenderTarget.resolveTarget = ResolveDepthRT;
		depthStencilRenderTarget.action = depthActions;
		depthStencilRenderTarget.exclusiveDepthStencil = inEDS;
		std::memset(&colorRenderTargets[1], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - 1));
	}

	inline int32 GetNumColorRenderTargets() const
	{
		int32 colorIndex = 0;
		for (; colorIndex < MaxSimultaneousRenderTargets; ++colorIndex)
		{
			const ColorEntry& Entry = colorRenderTargets[colorIndex];
			if (!Entry.renderTarget)
			{
				break;
			}
		}

		return colorIndex;
	}

	inline bool IsMSAA() const
	{
		return m_IsMSAA;
	}

	void Validate() const;

	void ConvertToRenderTargetsInfo(RHISetRenderTargetsInfo& OutRTInfo) const;

public:
	int32 uavIndex = -1;
	int32 numUAVs = 0;
	UnorderedAccessViewRHIRef uAVs[MaxSimultaneousUAVs];

private:
	bool m_IsMSAA = false;
};
