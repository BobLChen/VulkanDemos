#pragma once

#include "Common/Common.h"
#include "Configuration/Platform.h"
#include "Application/GenericWindow.h"

#include <string>
#include <X11/Xutil.h>

class LinuxApplication;

class LinuxWindow : public GenericWindow
{
public:

	virtual ~LinuxWindow();

	static std::shared_ptr<LinuxWindow> Make(int32 width, int32 height, const char* title);

	float GetAspectRatio() const;

	void Initialize(LinuxApplication* const application);

	virtual float GetDPIScaleFactor() const override;

	virtual void SetDPIScaleFactor(float value) override;

	virtual void ReshapeWindow(int32 x, int32 y, int32 width, int32 height) override;

	virtual bool GetFullScreenInfo(int32& x, int32& y, int32& width, int32& height) const override;

	virtual void MoveWindowTo(int32 x, int32 y) override;

	virtual void BringToFront(bool force = false) override;

	virtual void Destroy() override;

	virtual void Minimize() override;

	virtual void Maximize() override;

	virtual void Restore() override;

	virtual void Show() override;

	virtual void Hide() override;

	virtual void SetWindowMode(WindowMode::Type windowMode) override;

	virtual bool IsMaximized() const override;

	virtual bool IsMinimized() const override;

	virtual bool IsVisible() const override;

	virtual bool GetRestoredDimensions(int32& x, int32& y, int32& width, int32& height) override;

	virtual void SetWindowFocus() override;

	virtual void SetOpacity(const float opacity) override;

	virtual void Enable(bool enable) override;

	virtual bool IsPointInWindow(int32 x, int32 y) const override;

	virtual int32 GetWindowBorderSize() const override;

	virtual int32 GetWindowTitleBarSize() const override;

	virtual bool IsForegroundWindow() const override;

	virtual void SetText(const char* const text) override;

	virtual void* GetOSWindowHandle() const override;

	virtual void CreateVKSurface(VkInstance instance, VkSurfaceKHR* outSurface) override;

	virtual const char** GetRequiredInstanceExtensions(uint32_t* count) override;

	virtual WindowMode::Type GetWindowMode() const override
	{
		return m_WindowMode;
	}

	FORCEINLINE xcb_connection_t* GetConnection() const
	{
		return m_Connection;
	}

	FORCEINLINE xcb_intern_atom_reply_t* GetAtomWmDeleteWindow() const
	{
		return m_AtomWmDeleteWindow;
	}
	
private:
	LinuxWindow(int32 width, int32 height, const char* title);

	float GetDPIScaleFactorAtPoint(float X, float Y);

private:
	std::string 				m_Title;
	xcb_window_t				m_Window;
	xcb_screen_t*				m_Screen;
	xcb_connection_t*			m_Connection;
	xcb_intern_atom_reply_t* 	m_AtomWmDeleteWindow;
	WindowMode::Type 			m_WindowMode;
	LinuxApplication* 			m_Application;
	bool 						m_Visible;
	float 						m_AspectRatio;
	float 						m_DPIScaleFactor;
	bool 						m_Minimized;
	bool 						m_Maximized;
};
