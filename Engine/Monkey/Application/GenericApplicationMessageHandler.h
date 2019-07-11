#pragma once

#include "Common/Common.h"
#include "Math/Vector2.h"

class GenericWindow;

namespace MouseButtons
{
	enum Type
	{
		Left = 0,
		Middle,
		Right,
		Thumb01,
		Thumb02,
		Invalid,
	};
}

struct WindowSizeLimits
{
public:

	WindowSizeLimits& SetMinWidth(float value)
	{ 
		minWidth = value;
		return *this;
	}

	WindowSizeLimits& SetMinHeight(float value)
	{ 
		minHeight = value;
		return *this;
	}

	WindowSizeLimits& SetMaxWidth(float value)
	{ 
		maxWidth = value;
		return *this;
	}

	WindowSizeLimits& SetMaxHeight(float value)
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

	virtual bool ShouldProcessUserInputMessages(const std::shared_ptr<GenericWindow> window) const
	{

		return false;
	}

	virtual bool OnKeyChar(const char character, const bool isRepeat)
	{

		return false;
	}

	virtual bool OnKeyDown(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
	{

		return false;
	}

	virtual bool OnKeyUp(const int32 keyCode, const uint32 characterCode, const bool isRepeat)
	{

		return false;
	}

	virtual bool OnMouseDown(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button)
	{

		return false;
	}

	virtual bool OnMouseDown(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button, const Vector2 cursorPos)
	{

		return false;
	}

	virtual bool OnMouseUp(const MouseButtons::Type button)
	{

		return false;
	}

	virtual bool OnMouseUp(const MouseButtons::Type button, const Vector2 cursorPos)
	{

		return false;
	}

	virtual bool OnMouseDoubleClick(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button)
	{

		return false;
	}

	virtual bool OnMouseDoubleClick(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button, const Vector2 cursorPos)
	{

		return false;
	}

	virtual bool OnMouseWheel(const float delta)
	{

		return false;
	}

	virtual bool OnMouseWheel(const float delta, const Vector2 cursorPos)
	{

		return false;
	}

	virtual bool OnMouseMove()
	{

		return false;
	}

	virtual bool OnRawMouseMove(const int32 x, const int32 y)
	{

		return false;
	}

	virtual bool OnCursorSet()
	{

		return false;
	}
	
	virtual bool OnTouchStarted(const std::shared_ptr<GenericWindow> window, const Vector2& location, float force, int32 touchIndex, int32 controllerId)
	{

		return false;
	}

	virtual bool OnTouchMoved(const Vector2& location, float force, int32 touchIndex, int32 controllerId)
	{

		return false;
	}

	virtual bool OnTouchEnded(const Vector2& location, int32 touchIndex, int32 controllerId)
	{

		return false;
	}

	virtual bool OnTouchForceChanged(const Vector2& location, float force, int32 touchIndex, int32 controllerId)
	{

		return false;
	}

	virtual bool OnTouchFirstMove(const Vector2& location, float force, int32 touchIndex, int32 controllerId)
	{

		return false;
	}

	virtual bool OnSizeChanged(const std::shared_ptr<GenericWindow> window, const int32 width, const int32 height, bool wasMinimized = false)
	{

		return false;
	}

	virtual void OnOSPaint(const std::shared_ptr<GenericWindow> window)
	{

	}

	virtual WindowSizeLimits GetSizeLimitsForWindow(const std::shared_ptr<GenericWindow> window) const
	{

		return WindowSizeLimits();
	}

	virtual void OnResizingWindow(const std::shared_ptr<GenericWindow> window)
	{

	}

	virtual bool BeginReshapingWindow(const std::shared_ptr<GenericWindow> window)
	{

		return true;
	}

	virtual void FinishedReshapingWindow(const std::shared_ptr<GenericWindow> window)
	{

	}

	virtual void HandleDPIScaleChanged(const std::shared_ptr<GenericWindow> window)
	{

	}

	virtual void SignalSystemDPIChanged(const std::shared_ptr<GenericWindow> window)
	{

	}

	virtual void OnMovedWindow(const std::shared_ptr<GenericWindow> window, const int32 x, const int32 y)
	{

	}

	virtual void OnWindowClose(const std::shared_ptr<GenericWindow> window)
	{

	}

	virtual void OnRequestingExit()
	{

	}

};