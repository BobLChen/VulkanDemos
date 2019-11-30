#pragma once

#include "Common/Common.h"

#include <vector>
#include <mutex>
#include <condition_variable>

class TaskThread;
class ThreadTask;

class TaskThreadPool
{
public:

	TaskThreadPool();

	virtual ~TaskThreadPool();

	virtual bool Create(uint32 numThreads);

	virtual void Destroy();

	virtual void AddTask(ThreadTask* task);

	virtual bool RetractTask(ThreadTask* task);

	virtual ThreadTask* ReturnToPoolOrGetNextJob(TaskThread* thread);

	static TaskThreadPool* Allocate();

	int32 GetNumQueuedJobs() const
	{
		return m_QueuedTask.size();
	}

	int32 GetNumThreads() const
	{
		return m_AllThreads.size();
	}

protected:

	std::vector<ThreadTask*>		m_QueuedTask;
	std::vector<TaskThread*>		m_QueuedThreads;
	std::vector<TaskThread*>		m_AllThreads;

	std::mutex						m_SynchMutex;
	bool							m_TimeToDie = false;

};