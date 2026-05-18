#pragma once
#include "../vk/vk.hpp"
#include "./wsi_desktop.hpp"
#include "wsi.hpp"

#ifdef X_SYSTEM_DESKTOP
X_BEGIN

struct xUpdateWindowNode : xListNode {};
using xWindowUpdateList = xList<xUpdateWindowNode>;

class xDesktopWindow
	: public xWindowStateNode
	, public xUpdateWindowNode {

public:
	X_MEMBER bool Init(const xWindowSettings & Settings);
	X_MEMBER void Clean();

	X_INLINE bool IsActive() const { return State == eWindowState::WS_ACTIVE; }
	X_INLINE bool IsClosed() const { return !NativeHandle; }
	X_INLINE bool IsFullScreen() const { return FullScreen; }

	X_MEMBER void SetCursorMode(eWindowCursorMode Mode);

public:
	X_MEMBER void OnCreated();
	X_MEMBER void OnClose();
	X_MEMBER void OnUpdate();
	X_MEMBER void OnRefresh();
	X_MEMBER void OnResized(size_t W, size_t H);
	X_MEMBER void OnContentScaleUpdated(float ScaleX, float ScaleY);
	X_MEMBER void OnCursorMove(double X, double Y);

private:
	bool CreateRenderer();
	void DestroyRenderer();

private:
	uint64_t WindowId     = {};
	uint64_t RendererId   = {};
	void *   NativeHandle = nullptr;
	void *   FullScreen   = nullptr;

private:
	friend xWindowHandle CreateWindow(const xWindowSettings & Settings);
	friend void          CleanupDyingWindows();
};

X_PRIVATE void DeferKillAllActiveWindows();
X_PRIVATE void CleanupDyingWindows();

X_END
#endif
