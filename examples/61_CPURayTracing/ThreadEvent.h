#pragma once

#include "Common/Common.h"

#include <mutex>
#include <condition_variable>

class ThreadEvent
{
public:
	enum TriggerType
	{
		TRIGGERED_NONE,
		TRIGGERED_ONE,
		TRIGGERED_ALL,
	};

public:

	ThreadEvent(bool isManualReset = false);

	virtual ~ThreadEvent();

	void Trigger();

	void Reset();

	bool Wait(uint32 waitTime = (uint32)-1);

	bool IsInitialized()
	{
		return m_Initialized;
	}

	bool IsManualReset()
	{
		return m_IsManualReset;
	}

	int32 WaitingThreads()
	{
		return m_WaitingThreads;
	}

	bool Wait()
	{
		return Wait((uint32)-1);
	}

private:

	FORCEINLINE void Lock()
	{
		m_Mutex.lock();
	}

	FORCEINLINE void Unlock()
	{
		m_Mutex.unlock();
	}

private:

	bool					m_Initialized;
	bool					m_IsManualReset;

	volatile TriggerType	m_Triggered;
	volatile int32			m_WaitingThreads;

	std::mutex				m_Mutex;
	std::condition_variable	m_Condition;

};