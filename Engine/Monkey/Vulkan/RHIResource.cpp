#include "RHIResource.h"

void RHIComputeShader::UpdateStats()
{
	PipelineStateStats::UpdateStats(m_Stats);
}