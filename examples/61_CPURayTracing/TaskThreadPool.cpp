#include "TaskThreadPool.h"
#include "TaskThread.h"
#include "ThreadTask.h"

TaskThreadPool::TaskThreadPool()
{
	
}

TaskThreadPool::~TaskThreadPool()
{
	Destroy();
}

bool TaskThreadPool::Create(uint32 numThreads)
{
	bool success = true;

	std::lock_guard<std::mutex> lock(m_SynchMutex);

	for (int32 i = 0; i < numThreads; ++i) 
	{
		TaskThread* thread = new TaskThread();
		if (thread->Create(this)) 
		{
			m_AllThreads.push_back(thread);
			m_QueuedThreads.push_back(thread);
		}
		else 
		{
			success = false;
			break;
		}
	}

	if (success == false) {
		Destroy();
	}

	return success;
}

void TaskThreadPool::Destroy()
{
	{
		std::lock_guard<std::mutex> lock(m_SynchMutex);

		m_TimeToDie = true;

		for (int32 i = 0; i < m_QueuedTask.size(); ++i) {
			m_QueuedTask[i]->Abandon();
		}

		m_QueuedTask.clear();
	}

	while (true)
	{
		std::lock_guard<std::mutex> lock(m_SynchMutex);

		if (m_AllThreads.size() == m_QueuedThreads.size()) {
			break;
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_SynchMutex);

		for (int32 i = 0; i < m_AllThreads.size(); ++i)
		{
			m_AllThreads[i]->KillThread();
			delete m_AllThreads[i];
		}

		m_AllThreads.clear();
		m_QueuedThreads.clear();
	}
}

void TaskThreadPool::AddTask(ThreadTask* task)
{
	if (m_TimeToDie) {
		task->Abandon();
		return;
	}

	TaskThread* thread = nullptr;

	{
		std::lock_guard<std::mutex> lock(m_SynchMutex);

		int32 availableThreadCount = m_QueuedThreads.size();
		if (availableThreadCount == 0) {
			m_QueuedTask.push_back(task);
			return;
		}

		int32 threadIndex = availableThreadCount - 1;
		thread = m_QueuedThreads[threadIndex];
		m_QueuedThreads.pop_back();
	}

	thread->DoWork(task);
}

bool TaskThreadPool::RetractTask(ThreadTask* task)
{
	if (m_TimeToDie) {
		return false;
	}

	std::lock_guard<std::mutex> lock(m_SynchMutex);

	bool retracted = false;

	for (int32 i = 0; i < m_QueuedTask.size(); ++i)
	{
		if (m_QueuedTask[i] == task)
		{
			retracted = true;
			m_QueuedTask.erase(m_QueuedTask.begin() + i);
			break;
		}
	}

	return retracted;
}

ThreadTask* TaskThreadPool::ReturnToPoolOrGetNextJob(TaskThread* thread)
{
	ThreadTask* task = nullptr;

	std::lock_guard<std::mutex> lock(m_SynchMutex);

	if (m_QueuedTask.size() > 0)
	{
		task = m_QueuedTask.back();
		m_QueuedTask.pop_back();
	}

	if (task == nullptr) {
		m_QueuedThreads.push_back(thread);
	}

	return task;
}

TaskThreadPool* TaskThreadPool::Allocate()
{
	return new TaskThreadPool();
}