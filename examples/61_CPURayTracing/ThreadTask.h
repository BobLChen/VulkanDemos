#pragma once

class ThreadTask
{
public:

	ThreadTask()
	{

	}

	virtual ~ThreadTask() 
	{

	}

	virtual void DoThreadedWork() = 0;

	virtual void Abandon() = 0;

};