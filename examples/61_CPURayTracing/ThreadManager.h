#pragma once

#include "Common/Common.h"

#include <map>
#include <mutex>

class RunnableThread;

class ThreadManager
{
public:

	void AddThread(RunnableThread* thread);

	void RemoveThread(RunnableThread* thread);

	const std::string& GetThreadName(uint64 threadID);

	static ThreadManager& Get();

private:

	std::map<uint64, RunnableThread*>	m_Threads;
	std::mutex							m_Mutex;

};