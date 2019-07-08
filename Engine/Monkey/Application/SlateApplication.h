#pragma once

#include "Common/Common.h"
#include "Math/Vector2.h"

#include "Application/GenericApplication.h"
#include "Application/GenericWindow.h"
#include "Application/GenericApplicationMessageHandler.h"

class Engine;

class SlateApplication : public GenericApplicationMessageHandler
{
public:

	SlateApplication();

	virtual ~SlateApplication();

	void Tick(float detalTime);

	void PumpMessages();
	
	void Init(Engine* engine);

	void Shutdown(bool shutdownPlatform);

	std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title);

	std::shared_ptr<GenericApplication> GetPlatformApplication();
    
    std::shared_ptr<GenericWindow> GetPlatformWindow();
    
	void SetCursorPos(const Vector2& mouseCoordinate);

public:

	virtual bool ShouldProcessUserInputMessages(const std::shared_ptr<GenericWindow> window) const override;

	virtual bool OnKeyChar(const char character, const bool isRepeat) override;

	virtual bool OnKeyDown(const int32 keyCode, const uint32 characterCode, const bool isRepeat) override;

	virtual bool OnKeyUp(const int32 keyCode, const uint32 characterCode, const bool isRepeat) override;

	virtual bool OnMouseDown(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button) override;

	virtual bool OnMouseDown(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button, const Vector2 cursorPos) override;

	virtual bool OnMouseUp(const MouseButtons::Type button) override;

	virtual bool OnMouseUp(const MouseButtons::Type button, const Vector2 cursorPos) override;

	virtual bool OnMouseDoubleClick(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button) override;

	virtual bool OnMouseDoubleClick(const std::shared_ptr<GenericWindow> window, const MouseButtons::Type button, const Vector2 cursorPos) override;

	virtual bool OnMouseWheel(const float delta) override;

	virtual bool OnMouseWheel(const float delta, const Vector2 cursorPos) override;

	virtual bool OnMouseMove() override;

	virtual bool OnRawMouseMove(const int32 x, const int32 y) override;

	virtual bool OnCursorSet() override;

	virtual bool OnTouchStarted(const std::shared_ptr<GenericWindow> window, const Vector2& location, float force, int32 touchIndex, int32 controllerId) override;

	virtual bool OnTouchMoved(const Vector2& location, float force, int32 touchIndex, int32 controllerId) override;

	virtual bool OnTouchEnded(const Vector2& location, int32 touchIndex, int32 controllerId) override;

	virtual bool OnTouchForceChanged(const Vector2& location, float force, int32 touchIndex, int32 controllerId) override;

	virtual bool OnTouchFirstMove(const Vector2& location, float force, int32 touchIndex, int32 controllerId) override;

	virtual bool OnSizeChanged(const std::shared_ptr<GenericWindow> window, const int32 width, const int32 height, bool wasMinimized = false) override;

	virtual void OnOSPaint(const std::shared_ptr<GenericWindow> window) override;

	virtual WindowSizeLimits GetSizeLimitsForWindow(const std::shared_ptr<GenericWindow> window) const override;

	virtual void OnResizingWindow(const std::shared_ptr<GenericWindow> window) override;

	virtual bool BeginReshapingWindow(const std::shared_ptr<GenericWindow> window) override;

	virtual void FinishedReshapingWindow(const std::shared_ptr<GenericWindow> window) override;

	virtual void SignalSystemDPIChanged(const std::shared_ptr<GenericWindow> window) override;

	virtual void HandleDPIScaleChanged(const std::shared_ptr<GenericWindow> window) override;

	virtual void OnMovedWindow(const std::shared_ptr<GenericWindow> window, const int32 x, const int32 y) override;

	virtual void OnWindowClose(const std::shared_ptr<GenericWindow> window) override;

	virtual void OnRequestingExit() override;

protected:

	void TickPlatform(float deltaTime);

private:

	typedef std::shared_ptr<GenericApplication>     GenericApplicationRef;
    typedef std::shared_ptr<GenericWindow>          GenericWindowRef;
    
	Engine*					m_Engine;
    GenericWindowRef        m_Window;
	GenericApplicationRef	m_Application;
};
