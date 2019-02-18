#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

struct PipelineStateStats
{
	PipelineStateStats()
		: firstFrameUsed(-1)
		, lastFrameUsed(-1)
		, createCount(0)
		, totalBindCount(0)
		, psoHash(0)
	{

	}

	~PipelineStateStats()
	{

	}

	static void UpdateStats(PipelineStateStats* status);

	int64 firstFrameUsed;
	int64 lastFrameUsed;
	uint64 createCount;
	int64 totalBindCount;
	uint32 psoHash;
};
