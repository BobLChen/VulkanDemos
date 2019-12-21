#pragma once

#include "Common/Common.h"

class RunnableThread;

class Runnable
{
public:

	Runnable()
		: runnableThread(nullptr)
	{

	}

	virtual ~Runnable()
	{

	}

	virtual bool Init()
	{
		return true;
	}

	virtual int32 Run() = 0;

	virtual void Stop()
	{

	}

	virtual void Exit()
	{

	}

protected:

	friend class RunnableThread;

	RunnableThread* runnableThread;

};
