#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/AppModeBase.h"

class TriangleMode : public AppModeBase
{
public:
	TriangleMode(int width, int height, const char* title)
		: AppModeBase(width, height, title)
	{

	}

	virtual ~TriangleMode()
	{

	}

	virtual void PreInit() override
	{
		
	}

	virtual void Init() override
	{

	}

	virtual void Loop() override
	{

	}

	virtual void Exist() override
	{

	}
};

AppModeBase* CreateAppMode(const char* cmdLine, int32 cmdShow)
{
	return new TriangleMode(1600, 900, "Triangle");
}