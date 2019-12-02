#pragma once

#include "Runnable.h"
#include "ThreadEvent.h"

class ThreadTask;
class TaskThreadPool;
class RunnableThread;

class TaskThread : public Runnable
{
public:

	TaskThread();

	virtual ~TaskThread();

	virtual bool Create(TaskThreadPool* pool);

	virtual bool KillThread();

	void DoWork(ThreadTask* task);

protected:

	virtual int32 Run() override;

protected:

	ThreadEvent*			m_DoWorkEvent;
	volatile bool			m_TimeToDie;
	ThreadTask* volatile	m_Task;
	TaskThreadPool*			m_OwningThreadPool;
	RunnableThread*			m_Thread;

};