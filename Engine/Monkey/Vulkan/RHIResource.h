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

	void SetReferencedTexture(RHITexture* InTexture)
	{
		m_ReferencedTexture = InTexture;
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

	void SetReferencedTexture(RHITexture* InTexture)
	{
		RHITextureReference::SetReferencedTexture(InTexture);
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