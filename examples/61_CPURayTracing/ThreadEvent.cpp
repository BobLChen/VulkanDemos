#include "ThreadEvent.h"

ThreadEvent::ThreadEvent(bool isManualReset)
	: m_Initialized(true)
	, m_IsManualReset(isManualReset)
	, m_Triggered(TRIGGERED_NONE)
	, m_WaitingThreads(0)
{

}

ThreadEvent::~ThreadEvent()
{
	if (m_Initialized)
	{
		Lock();
		m_IsManualReset = true;
		Unlock();

		Trigger();

		Lock();
		m_Initialized = false;
		while (m_WaitingThreads > 0)
		{
			Unlock();
			Lock();
		}
		Unlock();
	}
}

void ThreadEvent::Trigger()
{
	Lock();

	if (m_IsManualReset)
	{
		m_Triggered = TRIGGERED_ALL;
		m_Condition.notify_all();
	}
	else
	{
		m_Triggered = TRIGGERED_ONE;
		m_Condition.notify_one();
	}

	Unlock();
}

void ThreadEvent::Reset()
{
	Lock();
	m_Triggered = TRIGGERED_NONE;
	Unlock();
}

bool ThreadEvent::Wait(uint32 waitTime)
{
	std::unique_lock<std::mutex> uniqueLock(m_Mutex);

	bool needWaiting = true;

	do
	{
		if (m_Triggered == TRIGGERED_ONE)
		{
			m_Triggered = TRIGGERED_NONE;
			needWaiting = false;
		}
		else if (m_Triggered == TRIGGERED_ALL)
		{
			needWaiting = false;
		}
		else if (waitTime == (uint32)-1)
		{
			m_WaitingThreads += 1;
			m_Condition.wait(uniqueLock);
			m_WaitingThreads -= 1;
		}
		else
		{
			m_WaitingThreads += 1;
			m_Condition.wait_for(uniqueLock, std::chrono::microseconds(waitTime));
			m_WaitingThreads -= 1;
		}
	} 
	while (needWaiting);

	return !needWaiting;
}