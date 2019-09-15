#pragma once

#include "Common/Common.h"
#include "Math/Vector2.h"

#include "Application/GenericApplication.h"
#include "Application/GenericWindow.h"
#include "Application/GenericApplicationMessageHandler.h"

class Engine;

class Application : public GenericApplicationMessageHandler
{
public:

	Application();

	virtual ~Application();

	void Tick(float time, float delta);

	void PumpMessages();
	
	void Init(Engine* engine);

	void Shutdown(bool shutdownPlatform);

	std::shared_ptr<GenericWindow> MakeWindow(int32 width, int32 height, const char* title);

	std::shared_ptr<GenericApplication> GetPlatformApplication();
    
    std::shared_ptr<GenericWindow> GetPlatformWindow();
    
	void SetCursorPos(const Vector2& mouseCoordinate);

public:
    
	virtual bool OnKeyDown(const KeyboardType key) override;

	virtual bool OnKeyUp(const KeyboardType key) override;
    
	virtual bool OnMouseDown(const MouseType type, const Vector2& pos) override;
    
	virtual bool OnMouseUp(const MouseType type, const Vector2& pos) override;
    
	virtual bool OnMouseDoubleClick(const MouseType type, const Vector2& pos) override;
    
	virtual bool OnMouseWheel(const float delta, const Vector2& pos) override;
    
    virtual bool OnMouseMove(const Vector2& pos) override;
    
	virtual bool OnTouchStarted(const std::vector<Vector2>& locations) override;

	virtual bool OnTouchMoved(const std::vector<Vector2>& locations) override;

	virtual bool OnTouchEnded(const std::vector<Vector2>& locations) override;

	virtual bool OnTouchForceChanged(const std::vector<Vector2>& locations) override;

	virtual bool OnTouchFirstMove(const std::vector<Vector2>& locations) override;
    
	virtual bool OnSizeChanged(const int32 width, const int32 height) override;
    
	virtual void OnOSPaint() override;
    
	virtual WindowSizeLimits GetSizeLimitsForWindow() const override;
    
	virtual void OnResizingWindow() override;
    
	virtual bool BeginReshapingWindow() override;

	virtual void FinishedReshapingWindow() override;

	virtual void SignalSystemDPIChanged() override;
    
	virtual void HandleDPIScaleChanged() override;

	virtual void OnMovedWindow(const int32 x, const int32 y) override;

	virtual void OnWindowClose() override;
    
	virtual void OnRequestingExit() override;

protected:

	void TickPlatform(float time, float delta);

private:

	typedef std::shared_ptr<GenericApplication>     GenericApplicationRef;
    typedef std::shared_ptr<GenericWindow>          GenericWindowRef;
    
	Engine*					m_Engine;
    GenericWindowRef        m_Window;
	GenericApplicationRef	m_Application;
};
