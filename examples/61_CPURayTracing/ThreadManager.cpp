#include "ThreadManager.h"
#include "RunnableThread.h"

void ThreadManager::AddThread(RunnableThread* thread)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	
	auto it = m_Threads.find(thread->GetThreadID());
	if (it == m_Threads.end()) {
		m_Threads.insert(std::make_pair(thread->GetThreadID(), thread));
	}
}

void ThreadManager::RemoveThread(RunnableThread* thread)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	auto it = m_Threads.find(thread->GetThreadID());
	if (it != m_Threads.end()) {
		m_Threads.erase(thread->GetThreadID());
	}
}

const std::string& ThreadManager::GetThreadName(uint64 threadID)
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	return m_Threads[threadID]->GetThreadName();
}

ThreadManager& ThreadManager::Get()
{
	static ThreadManager manager;
	return manager;
}