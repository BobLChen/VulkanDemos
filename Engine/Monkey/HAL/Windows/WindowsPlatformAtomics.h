#pragma once

#include "Common/Common.h"

#include "HAL/GenericPlatformAtomics.h"

#include <intrin.h>

struct WindowsPlatformAtomics : public GenericPlatformAtomics
{
	static_assert(sizeof(int8)  == sizeof(char)      && alignof(int8)  == alignof(char),	  "int8 must be compatible with char");
	static_assert(sizeof(int16) == sizeof(short)     && alignof(int16) == alignof(short),	  "int16 must be compatible with short");
	static_assert(sizeof(int32) == sizeof(long)      && alignof(int32) == alignof(long),	  "int32 must be compatible with long");
	static_assert(sizeof(int64) == sizeof(long long) && alignof(int64) == alignof(long long), "int64 must be compatible with long long");

	static FORCEINLINE int8 InterlockedIncrement(volatile int8* value)
	{
		return (int8)_InterlockedExchangeAdd8((char*)value, 1) + 1;
	}

	static FORCEINLINE int16 InterlockedIncrement(volatile int16* value)
	{
		return (int16)_InterlockedIncrement16((short*)value);
	}

	static FORCEINLINE int32 InterlockedIncrement(volatile int32* value)
	{
		return (int32)::_InterlockedIncrement((long*)value);
	}

	static FORCEINLINE int64 InterlockedIncrement(volatile int64* value)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedIncrement64((long long*)value);
#else
		while (true)
		{
			int64 oldValue = *value;
			if (_InterlockedCompareExchange64(value, oldValue + 1, oldValue) == oldValue) {
				return oldValue + 1;
			}
		}
#endif
	}

	static FORCEINLINE int8 InterlockedDecrement(volatile int8* value)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)value, -1) - 1;
	}

	static FORCEINLINE int16 InterlockedDecrement(volatile int16* value)
	{
		return (int16)::_InterlockedDecrement16((short*)value);
	}

	static FORCEINLINE int32 InterlockedDecrement(volatile int32* value)
	{
		return (int32)::_InterlockedDecrement((long*)value);
	}

	static FORCEINLINE int64 InterlockedDecrement(volatile int64* value)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedDecrement64((long long*)value);
#else
		while (true)
		{
			int64 oldValue = *value;
			if (_InterlockedCompareExchange64(value, oldValue - 1, oldValue) == oldValue) {
				return oldValue - 1;
			}
		}
#endif
	}

	static FORCEINLINE int8 InterlockedAdd(volatile int8* value, int8 amount)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)value, (char)amount);
	}

	static FORCEINLINE int16 InterlockedAdd(volatile int16* value, int16 amount)
	{
		return (int16)::_InterlockedExchangeAdd16((short*)value, (short)amount);
	}

	static FORCEINLINE int32 InterlockedAdd(volatile int32* value, int32 amount)
	{
		return (int32)::_InterlockedExchangeAdd((long*)value, (long)amount);
	}

	static FORCEINLINE int64 InterlockedAdd(volatile int64* value, int64 amount)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedExchangeAdd64((int64*)value, (int64)amount);
#else
		while (true)
		{
			int64 oldValue = *value;
			if (_InterlockedCompareExchange64(value, oldValue + amount, oldValue) == oldValue) {
				return oldValue + amount;
			}
		}
#endif
	}

	static FORCEINLINE int8 InterlockedExchange(volatile int8* value, int8 exchange)
	{
		return (int8)::_InterlockedExchange8((char*)value, (char)exchange);
	}

	static FORCEINLINE int16 InterlockedExchange(volatile int16* value, int16 exchange)
	{
		return (int16)::_InterlockedExchange16((short*)value, (short)exchange);
	}

	static FORCEINLINE int32 InterlockedExchange(volatile int32* value, int32 exchange)
	{
		return (int32)::_InterlockedExchange((long*)value, (long)exchange);
	}

	static FORCEINLINE int64 InterlockedExchange(volatile int64* value, int64 exchange)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedExchange64((long long*)value, (long long)exchange);
#else
		while (true)
		{
			int64 oldValue = *value;
			if (_InterlockedCompareExchange64(value, exchange, oldValue) == oldValue) {
				return oldValue;
			}
		}
#endif
	}

	static FORCEINLINE void* InterlockedExchangePtr(void** dest, void* exchange)
	{
#if PLATFORM_64BITS
		return (void*)::_InterlockedExchange64((int64*)(dest), (int64)(exchange));
#else
		return (void*)::_InterlockedExchange((long*)(dest), (long)(exchange));
#endif
	}

	static FORCEINLINE int8 InterlockedCompareExchange(volatile int8* dest, int8 exchange, int8 comparand)
	{
		return (int8)::_InterlockedCompareExchange8((char*)dest, (char)exchange, (char)comparand);
	}

	static FORCEINLINE int16 InterlockedCompareExchange(volatile int16* dest, int16 exchange, int16 comparand)
	{
		return (int16)::_InterlockedCompareExchange16((short*)dest, (short)exchange, (short)comparand);
	}

	static FORCEINLINE int32 InterlockedCompareExchange(volatile int32* dest, int32 exchange, int32 comparand)
	{
		return (int32)::_InterlockedCompareExchange((long*)dest, (long)exchange, (long)comparand);
	}

	static FORCEINLINE int64 InterlockedCompareExchange(volatile int64* dest, int64 exchange, int64 comparand)
	{
		return (int64)::_InterlockedCompareExchange64(dest, exchange, comparand);
	}

	static FORCEINLINE int8 AtomicRead(volatile const int8* src)
	{
		return InterlockedCompareExchange((int8*)src, 0, 0);
	}

	static FORCEINLINE int16 AtomicRead(volatile const int16* src)
	{
		return InterlockedCompareExchange((int16*)src, 0, 0);
	}

	static FORCEINLINE int32 AtomicRead(volatile const int32* src)
	{
		return InterlockedCompareExchange((int32*)src, 0, 0);
	}

	static FORCEINLINE int64 AtomicRead(volatile const int64* src)
	{
		return InterlockedCompareExchange((int64*)src, 0, 0);
	}

	static FORCEINLINE int8 AtomicRead_Relaxed(volatile const int8* src)
	{
		return *src;
	}

	static FORCEINLINE int16 AtomicRead_Relaxed(volatile const int16* src)
	{
		return *src;
	}

	static FORCEINLINE int32 AtomicRead_Relaxed(volatile const int32* src)
	{
		return *src;
	}

	static FORCEINLINE int64 AtomicRead_Relaxed(volatile const int64* src)
	{
#if PLATFORM_64BITS
		return *src;
#else
		return InterlockedCompareExchange((volatile int64*)src, 0, 0);
#endif
	}

	static FORCEINLINE void AtomicStore(volatile int8* src, int8 val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore(volatile int16* src, int16 val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore(volatile int32* src, int32 val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore(volatile int64* src, int64 val)
	{
		InterlockedExchange(src, val);
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int8* src, int8 val)
	{
		*src = val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int16* src, int16 val)
	{
		*src = val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int32* src, int32 val)
	{
		*src = val;
	}

	static FORCEINLINE void AtomicStore_Relaxed(volatile int64* src, int64 val)
	{
#if PLATFORM_64BITS
		*src = val;
#else
		InterlockedExchange(src, val);
#endif
	}

	static FORCEINLINE void* InterlockedCompareExchangePointer(void** dest, void* exchange, void* comparand)
	{
#if PLATFORM_64BITS
		return (void*)::_InterlockedCompareExchange64((int64*)dest, (int64)exchange, (int64)comparand);
#else
		return (void*)::_InterlockedCompareExchange((long*)dest, (long)exchange, (long)comparand);
#endif
	}
	
};

typedef WindowsPlatformAtomics PlatformAtomics;