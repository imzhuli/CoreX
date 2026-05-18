#include "./window_desktop.hpp"

#include "../core/indexed_storage_static.hpp"
#include "../renderer/renderer.hpp"
#include "../vk/vk.hpp"
#include "../vk/vk_debug.hpp"
#include "../xengine/engine.hpp"
#include "./darwin/osx.h"
#include "GLFW/glfw3.h"
#include "wsi.hpp"

#ifdef X_SYSTEM_DESKTOP
X_BEGIN

static xWindowStateList  ActiveWindowList;
static xWindowStateList  DyingWindowList;
static xWindowUpdateList UpdateWindowList;

static xIndexedStorageStatic<xDesktopWindow, 128> DesktopWindowPool;

static void window_close_callback(GLFWwindow * window) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
	WindowPtr->OnClose();
}

static void window_refresh_callback(GLFWwindow * window) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
	WindowPtr->OnRefresh();
}

static void window_resize_callback(struct GLFWwindow * window, int w, int h) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
	ssize32_t width  = (ssize32_t)w;
	ssize32_t height = (ssize32_t)h;
	WindowPtr->OnResized(width, height);
}

static void window_content_scale_callback(GLFWwindow * window, float xscale, float yscale) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
	WindowPtr->OnContentScaleUpdated(xscale, yscale);
}

static void window_cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
	WindowPtr->OnCursorMove(xpos, ypos);
}

static void window_disabled_cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
	WindowPtr->OnCursorMove(xpos, ypos);
	glfwSetCursorPos(window, 0, 0);
}

static void window_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	if (!WindowPtr->IsActive()) {
		return;
	}
}

xWindowHandle CreateWindow(const xWindowSettings & Settings) {
	auto WindowId = DesktopWindowPool.Acquire();
	if (!WindowId) {
		return {};
	}
	auto WindowPtr      = &DesktopWindowPool[WindowId];
	WindowPtr->WindowId = WindowId;

	auto WindowGuard = xScopeGuard([&] { DesktopWindowPool.Release(WindowId); });

	if (!WindowPtr->Init(Settings)) {
		return {};
	}
	WindowGuard.Dismiss();

	WindowPtr->OnCreated();
	return { WindowId };
}

void DeferDestroyWindow(xWindowHandle Handle) {
	auto W = DesktopWindowPool.CheckAndGet(Handle.WindowId);
	if (!W || W->State == eWindowState::WS_DYING) {
		return;
	}
	assert(W->State == eWindowState::WS_ACTIVE);
	W->State = eWindowState::WS_DYING;
	DyingWindowList.GrabTail(*W);
}

bool xDesktopWindow::Init(const xWindowSettings & Settings) {
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, Settings.Resizable ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_DECORATED, !Settings.Borderless ? GLFW_TRUE : GLFW_FALSE);
#ifdef TARGET_OS_MAC
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif

	auto Monitor = (Settings.WindowMode == eWindowMode::FullScreen) ? glfwGetPrimaryMonitor() : nullptr;

	auto win = glfwCreateWindow(
		Settings.Size.Width, Settings.Size.Height, "XGameEngine", (Settings.WindowMode == eWindowMode::FullScreen ? glfwGetPrimaryMonitor() : nullptr), nullptr
	);
	if (!win) {
		return false;
	}

	NativeHandle = win;
	State        = eWindowState::WS_ACTIVE;
	ActiveWindowList.AddTail(*this);
	UpdateWindowList.AddTail(*this);
	auto HandleGuard = xScopeGuard([&, this] {
		glfwDestroyWindow(win);
		xWindowStateList::Remove(*this);
		xWindowUpdateList::Remove(*this);
		Reset(State, eWindowState::WS_ACTIVE);
		Reset(NativeHandle);
	});

	FullScreen = Monitor;
	if (Settings.PositionMode == eWindowPositionMode::Centered) {
		auto PrimaryMonitor = glfwGetPrimaryMonitor();
		if (!PrimaryMonitor) {
			return false;
		}
		auto desktop = glfwGetVideoMode(PrimaryMonitor);
		if (!desktop) {
			return false;
		}
		glfwSetWindowPos(win, ((int32_t)desktop->width - Settings.Size.Width) / 2, ((int32_t)desktop->height - Settings.Size.Height) / 2);
	} else {
		glfwSetWindowPos(win, 0, 0);
	}

	glfwSetWindowUserPointer(win, this);
	glfwSetWindowRefreshCallback(win, &window_refresh_callback);
	glfwSetWindowSizeCallback(win, &window_resize_callback);
	glfwSetWindowContentScaleCallback(win, &window_content_scale_callback);
	glfwSetKeyCallback(win, &window_key_callback);
	glfwSetCursorPosCallback(win, &window_cursor_position_callback);
	glfwSetWindowCloseCallback(win, &window_close_callback);
	Settings.Hidden ? Pass() : glfwShowWindow(win);

	if (!CreateRenderer()) {
		return false;
	}

	HandleGuard.Dismiss();
	return true;
}

bool xDesktopWindow::CreateRenderer() {
	auto Window        = (GLFWwindow *)NativeHandle;
	auto SurfaceHandle = (VkSurfaceKHR)VK_NULL_HANDLE;
	auto SurfaceResult = glfwCreateWindowSurface(VulkanInstance, Window, nullptr, &SurfaceHandle);
	if (SurfaceResult != VK_SUCCESS) {
		return false;
	}
	if (!(RendererId = xRenderer::Create(std::move(SurfaceHandle)))) {
		return false;
	}
	return true;
}

void xDesktopWindow::OnCreated() {
	auto win = (GLFWwindow *)NativeHandle;
	int  w, h;
	glfwGetWindowSize(win, &w, &h);
	OnResized(w, h);
	float rx, ry;
	glfwGetWindowContentScale(win, &rx, &ry);
	OnContentScaleUpdated(rx, ry);
}

void xDesktopWindow::Clean() {
	auto Window = (GLFWwindow *)Steal(NativeHandle);
	assert(Window);
	xRenderer::Destroy(Steal(RendererId));
	glfwDestroyWindow(Window);
}

void xDesktopWindow::SetCursorMode(eWindowCursorMode Mode) {
	auto Window = (GLFWwindow *)Steal(NativeHandle);
	switch (Mode) {
		case eWindowCursorMode::Native: {
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(Window, &window_cursor_position_callback);
		} break;

		case eWindowCursorMode::Hidden: {
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetCursorPosCallback(Window, &window_cursor_position_callback);
		} break;

		case eWindowCursorMode::Offset: {
			glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPosCallback(Window, &window_disabled_cursor_position_callback);
		} break;
	}
}

void xDesktopWindow::OnRefresh() {
}

void xDesktopWindow::OnResized(size_t Width, size_t Height) {
	// X_DEBUG_PRINTF("ClientAreaSize: %zi, %zi", Width, Height);
}

void xDesktopWindow::OnContentScaleUpdated(float xscale, float yscale) {
	// X_DEBUG_PRINTF("xscale=%f, yscale=%f", xscale, yscale);
}

void xDesktopWindow::OnCursorMove(double OffsetX, double OffsetY) {
}

void xDesktopWindow::OnUpdate() {
}

void xDesktopWindow::OnClose() {
	DeferDestroyWindow({ WindowId });
}

void UpdateWindows(uint64_t TimeoutMS) {
	glfwWaitEventsTimeout(TimeoutMS / 1000.);
}

void DeferKillAllActiveWindows() {
	DyingWindowList.GrabListTail(ActiveWindowList);
}

void CleanupDyingWindows() {
	while (auto P = (xDesktopWindow *)DyingWindowList.PopHead()) {
		P->Clean();
		DesktopWindowPool.Release(P->WindowId);
	}
	if (ActiveWindowList.IsEmpty()) {
		StopXEngine();
	}
}

X_END
#endif
