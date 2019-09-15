#pragma once

#include "Common/Common.h"
#include "Math/Vector2.h"
#include "GenericPlatform/InputManager.h"

class GenericWindow;

struct WindowSizeLimits
{
public:

	inline WindowSizeLimits& SetMinWidth(float value)
	{ 
		minWidth = value;
		return *this;
	}

	inline WindowSizeLimits& SetMinHeight(float value)
	{ 
		minHeight = value;
		return *this;
	}

	inline WindowSizeLimits& SetMaxWidth(float value)
	{ 
		maxWidth = value;
		return *this;
	}

	inline WindowSizeLimits& SetMaxHeight(float value)
	{ 
		maxHeight = value;
		return *this;
	}

public:
	float minWidth;
	float minHeight;
	float maxWidth;
	float maxHeight;
};

class GenericApplicationMessageHandler
{
public:

	GenericApplicationMessageHandler()
	{

	}

	virtual ~GenericApplicationMessageHandler() 
	{

	}
    
	virtual bool OnKeyDown(const KeyboardType key)
	{
        
		return false;
	}
    
	virtual bool OnKeyUp(const KeyboardType key)
	{
        
		return false;
	}
    
	virtual bool OnMouseDown(MouseType type, const Vector2& pos)
	{
        
		return false;
	}
    
	virtual bool OnMouseUp(MouseType type, const Vector2& pos)
	{

		return false;
	}
    
	virtual bool OnMouseDoubleClick(MouseType type, const Vector2& pos)
	{

		return false;
	}
    
	virtual bool OnMouseWheel(const float delta, const Vector2& pos)
	{

		return false;
	}
    
    virtual bool OnMouseMove(const Vector2& pos)
    {
        return false;
    }
    
	virtual bool OnTouchStarted(const std::vector<Vector2>& locations)
	{

		return false;
	}
    
	virtual bool OnTouchMoved(const std::vector<Vector2>& locations)
	{

		return false;
	}

	virtual bool OnTouchEnded(const std::vector<Vector2>& locations)
	{

		return false;
	}
    
	virtual bool OnTouchForceChanged(const std::vector<Vector2>& locations)
	{

		return false;
	}
    
    virtual bool OnTouchFirstMove(const std::vector<Vector2>& locations)
	{
        
		return false;
	}
    
	virtual bool OnSizeChanged(const int32 width, const int32 height)
	{
        
		return false;
	}
    
	virtual void OnOSPaint()
	{
        
	}
    
	virtual WindowSizeLimits GetSizeLimitsForWindow() const
	{
		return WindowSizeLimits();
	}
    
	virtual void OnResizingWindow()
	{

	}

	virtual bool BeginReshapingWindow()
	{

		return true;
	}

	virtual void FinishedReshapingWindow()
	{
        
	}

	virtual void HandleDPIScaleChanged()
	{
        
	}

	virtual void SignalSystemDPIChanged()
	{
        
	}

	virtual void OnMovedWindow(const int32 x, const int32 y)
	{
        
	}

	virtual void OnWindowClose()
	{
        
	}

	virtual void OnRequestingExit()
	{
        
	}
};
