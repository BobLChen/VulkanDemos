#include "TaskThread.h"
#include "ThreadTask.h"
#include "TaskThreadPool.h"
#include "RunnableThread.h"

TaskThread::TaskThread()
	: m_DoWorkEvent(nullptr)
	, m_TimeToDie(false)
	, m_Task(nullptr)
	, m_OwningThreadPool(nullptr)
	, m_Thread(nullptr)
{
	
}

TaskThread::~TaskThread()
{

}

bool TaskThread::Create(TaskThreadPool* pool)
{
	static int32 TaskThreadIndex = 0;
	char buf[128];
	sprintf(buf, "TaskThread %d", TaskThreadIndex);
	TaskThreadIndex += 1;

	m_OwningThreadPool = pool;
	m_DoWorkEvent = new ThreadEvent();
	m_Thread = RunnableThread::Create(this, std::string(buf));

	return true;
}

bool TaskThread::KillThread()
{
	bool success = true;

	m_TimeToDie = true;
	m_DoWorkEvent->Trigger();
	m_Thread->WaitForCompletion();

	delete m_DoWorkEvent;
	delete m_Thread;

	return success;
}

void TaskThread::DoWork(ThreadTask* task)
{
	m_Task = task;
	m_DoWorkEvent->Trigger();
}

int32 TaskThread::Run()
{
	while (!m_TimeToDie)
	{
		bool continueWaiting = true;
		while (continueWaiting) {				
			continueWaiting = !(m_DoWorkEvent->Wait(5));
		}

		ThreadTask* localTask = m_Task;
		m_Task = nullptr;

		while (localTask != nullptr)
		{
			localTask->DoThreadedWork();
			localTask = m_OwningThreadPool->ReturnToPoolOrGetNextJob(this);
		} 
	}

	return 0;
}