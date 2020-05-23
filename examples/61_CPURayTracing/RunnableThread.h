#pragma once

#include "Common/Common.h"
#include "ThreadEvent.h"

#include <thread>
#include <string>

class Runnable;
class ThreadManager;

class RunnableThread
{
	friend class ThreadManager;

public:

	RunnableThread();

	virtual ~RunnableThread();

	static RunnableThread* Create(Runnable* runnable, const std::string& threadName);

	virtual void WaitForCompletion()
	{
		if (m_LocalThread->joinable()) {
			m_LocalThread->join();
		}
	}

	const uint64 GetThreadID() const
	{
		return m_ThreadID;
	}

	const std::string& GetThreadName() const
	{
		return m_ThreadName;
	}

protected:

	virtual bool CreateInternal(Runnable* runnable, const std::string& threadName);

	virtual void PreRun();

	virtual int32 Run();
	
	virtual void PostRun();

	static void ThreadFunction(void* pThis);

protected:

	std::string		m_ThreadName;
	Runnable*		m_Runnable;
	ThreadEvent*	m_InitSyncEvent;
	uint64			m_ThreadID;
	std::thread*	m_LocalThread;
};