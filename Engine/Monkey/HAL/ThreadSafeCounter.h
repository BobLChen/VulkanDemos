#pragma once

#include "Common/Common.h"
#include "HAL/PlatformAtomics.h"

class ThreadSafeCounter
{
public:
	typedef int32 IntegerType;

	ThreadSafeCounter()
	{
		m_Counter = 0;
	}

	ThreadSafeCounter(const ThreadSafeCounter& other)
	{
		m_Counter = other.GetValue();
	}

	ThreadSafeCounter(int32 value)
	{
		m_Counter = value;
	}

	int32 Increment()
	{
		return PlatformAtomics::InterlockedIncrement(&m_Counter);
	}

	int32 Add(int32 amount)
	{
		return PlatformAtomics::InterlockedAdd(&m_Counter, amount);
	}

	int32 Decrement()
	{
		return PlatformAtomics::InterlockedDecrement(&m_Counter);
	}

	int32 Subtract(int32 amount)
	{
		return PlatformAtomics::InterlockedAdd(&m_Counter, -amount);
	}

	int32 Set(int32 value)
	{
		return PlatformAtomics::InterlockedExchange(&m_Counter, value);
	}

	int32 Reset()
	{
		return PlatformAtomics::InterlockedExchange(&m_Counter, 0);
	}

	int32 GetValue() const
	{
		return PlatformAtomics::AtomicRead(&(const_cast<ThreadSafeCounter*>(this)->m_Counter));
	}
	
protected:

	void operator=(const ThreadSafeCounter& other)
	{

	}

protected:

	volatile int32 m_Counter;
};