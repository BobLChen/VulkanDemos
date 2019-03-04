#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

struct FCrc
{
	static uint32 g_CRCTablesSB8[8][256];

	static uint32 MemCrc32(const void* data, int32 length, uint32 crc = 0);

	static uint32 ReverseBits(uint32 bits);

	static uint32 StrCrc32(const uint32* data, uint32 crc = 0);

	static uint32 StrCrc32(const uint32* data, uint32 crc = 0);

};