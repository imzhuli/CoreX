#pragma once
#include "../core/core_min.hpp"
#include "../core/list.hpp"

#include <compare>

X_BEGIN

constexpr size32_t DefaultInitWindowWidth  = 1440;
constexpr size32_t DefaultInitWindowHeight = 900;

struct xNativeWindowHandle;
class xWindow;

enum struct eWindowMode {
	Normal,
	FullScreen,
};

enum struct eWindowPositionMode {
	Centered,
	TopLeft,
};

enum struct eWindowScaleMode {
	Keep,
	CanvasItem,
	Viewport,
};

enum struct eWindowCursorMode {
	Native = 0,
	Hidden = 1,
	Offset = 2,
};

struct xWindowSettings final {
	eWindowMode         WindowMode   = eWindowMode::Normal;
	eWindowPositionMode PositionMode = eWindowPositionMode::Centered;
	eWindowScaleMode    ScaleMode    = eWindowScaleMode::Keep;

	bool  Hidden     = false;
	bool  Borderless = false;
	bool  Resizable  = true;
	float Scale      = 1.0f;

	struct {
		int32_t X = 0;
		int32_t Y = 0;
	} Posotion;

	struct {
		uint32_t Width  = DefaultInitWindowWidth;
		uint32_t Height = DefaultInitWindowHeight;
	} Size;
};

class iWindow : xAbstract {
public:
	virtual bool Init(const xWindowSettings & Settings) { return false; }
	virtual void Clean() {}

	virtual bool IsClosed() { return true; }
	virtual bool IsFullScreen() { return false; }

	virtual void SetCursorMode(eWindowCursorMode Mode) {}

	// event processing
	virtual void OnCreated() {}
	virtual void OnRefresh() {}
	virtual void OnResized(size_t Width, size_t Height) {}
	virtual void OnCursorMove(double OffsetX, double OffsetY) {}
	virtual void OnClosed() {}
};

struct xWindowUpdateListNode : xListNode {};
using xWindowUpdateList = xList<xWindowUpdateListNode>;
struct xWindowLifeCycleListNode : xListNode {
	bool Active = true;
};
using xWindowActiveList  = xList<xWindowLifeCycleListNode>;
using xWindowDestroyList = xList<xWindowLifeCycleListNode>;

X_API bool InitWSI();
X_API void CleanWSI();

X_API iWindow * CreateWindow(const xWindowSettings & Settings = {});
X_API void      DeferDestroyWindow(iWindow * WindowPtr);
X_API void      WSILoopOnce(uint_fast32_t TimeoutMS = 1);
X_API bool      WSIHasOpenWindow();

X_END
