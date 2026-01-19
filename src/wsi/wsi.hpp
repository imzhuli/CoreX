#pragma once
#include "../core/core_min.hpp"
#include "../core/list.hpp"
#include "../xengine/base.hpp"

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

enum eWindowState : uint8_t {
	WS_INIT   = 0,
	WS_ACTIVE = 1,
	WS_DYING  = 2,
};

struct xWindowStateNode : xListNode {
	eWindowState State = WS_INIT;
};
using xWindowStateList = xList<xWindowStateNode>;

struct xNativeWindowHandle : xWindowStateNode {
	uint64_t  WindowId     = {};
	xVariable NativeHandle = {};
};

X_PRIVATE void UpdateWindows(uint64_t TimestampMS);

X_API bool InitWSI();
X_API void CleanWSI();

X_API xHandle CreateWindow(const xWindowSettings & Settings = {});
X_API void    DeferDestroyWindow(xHandle WindowPtr);

X_PRIVATE void WSILoopOnce(uint_fast32_t TimeoutMS = 1);
X_PRIVATE bool WSIHasDeferredCommands();
X_PRIVATE void WSIProcessedCommandQueue();

X_END
