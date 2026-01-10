#pragma once
#include "../vk/vk.hpp"
#include "./wsi_desktop.hpp"

#ifdef X_SYSTEM_DESKTOP
X_BEGIN

struct xNativeWindowHandle {
	GLFWwindow * Value = nullptr;
};

class xDesktopWindow
	: public iWindow
	, public xWindowUpdateListNode {
public:
	X_PRIVATE_MEMBER bool Init(const xWindowSettings & Settings) override;
	X_PRIVATE_MEMBER void Clean() override;

	X_PRIVATE_MEMBER void SetCursorMode(eWindowCursorMode Mode) override;
	X_PRIVATE_MEMBER bool IsClosed() override;
	X_PRIVATE_MEMBER bool IsFullScreen() override;

public:
	// event callbacks
	X_PRIVATE_MEMBER void OnCreated() override;
	X_PRIVATE_MEMBER void OnRefresh() override;
	X_PRIVATE_MEMBER void OnResized(size_t Width, size_t Height) override;
	X_PRIVATE_MEMBER void OnCursorMove(double OffsetX, double OffsetY) override;
	X_PRIVATE_MEMBER void OnClosed() override;

	X_PRIVATE_MEMBER void OnContentScaleUpdated(float xscale, float yscale);

	//
	X_PRIVATE_MEMBER void Update();
	X_PRIVATE_MEMBER void Close();

	//
protected:
	X_PRIVATE_MEMBER bool CreateRenderer();

protected:
	xNativeWindowHandle NativeHandle = {};

private:
	// shadow params
	bool FullScreen_ = false;
};

X_END
#endif
