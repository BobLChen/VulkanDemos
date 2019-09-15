#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include <string>

struct Crc
{
	static uint32 g_CRCTablesSB8[8][256];

	static uint32 MemCrc32(const void* data, int32 length, uint32 crc = 0);

	static uint32 ReverseBits(uint32 bits);

	static uint32 StrCrc32(const char* data, int32 length, uint32 crc = 0);

	template <class T>
	static void HashCombine(uint32 &seed, const T &v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
    
	static uint32 MakeHashCode(uint32 a, uint32 b)
	{
		uint32 seed = 0;
		HashCombine(seed, a);
		HashCombine(seed, b);
		return seed;
	}

	static uint32 MakeHashCode(uint32 a, uint32 b, uint32 c)
	{
		uint32 seed = 0;
		HashCombine(seed, a);
		HashCombine(seed, b);
		HashCombine(seed, c);
		return seed;
	}

	static uint32 MakeHashCode(uint32 a, uint32 b, uint32 c, uint32 d)
	{
		uint32 seed = 0;
		HashCombine(seed, a);
		HashCombine(seed, b);
		HashCombine(seed, c);
		HashCombine(seed, d);
		return seed;
	}
	
};
