#include "RunnableThread.h"
#include "ThreadManager.h"
#include "Runnable.h"

#include <sstream>

RunnableThread::RunnableThread()
	: m_ThreadName("")
	, m_Runnable(nullptr)
	, m_InitSyncEvent(nullptr)
	, m_ThreadID(0)
	, m_LocalThread(nullptr)
{

}

RunnableThread::~RunnableThread()
{
	ThreadManager::Get().RemoveThread(this);

	delete m_LocalThread;
}

RunnableThread* RunnableThread::Create(Runnable* runnable, const std::string& threadName)
{
	RunnableThread* thread = new RunnableThread();
	thread->CreateInternal(runnable, threadName);
	return thread;
}

bool RunnableThread::CreateInternal(Runnable* runnable, const std::string& threadName)
{
	m_Runnable		= runnable;
	m_ThreadName	= threadName;
	m_InitSyncEvent = new ThreadEvent();

	m_LocalThread   = new std::thread(RunnableThread::ThreadFunction, this);

	m_InitSyncEvent->Wait((uint32)-1);

	delete m_InitSyncEvent;
	m_InitSyncEvent = nullptr;

	return true;
}

void RunnableThread::ThreadFunction(void* pThis)
{
	RunnableThread* thisThread = (RunnableThread*)pThis;

	// ID
	{
		std::stringstream ss;
		ss << std::this_thread::get_id();
		thisThread->m_ThreadID = std::stoull(ss.str());
	}

	ThreadManager::Get().AddThread(thisThread);

	thisThread->PreRun();
	thisThread->Run();
	thisThread->PostRun();
}

void RunnableThread::PreRun()
{
	m_Runnable->runnableThread = this;
}

void RunnableThread::PostRun()
{
	m_Runnable->runnableThread = nullptr;
}

int32 RunnableThread::Run()
{
	int32 exitCode = 0;

	m_InitSyncEvent->Trigger();

	exitCode = m_Runnable->Run();

	m_Runnable->Exit();

	return exitCode;
}