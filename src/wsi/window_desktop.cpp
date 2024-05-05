#include "./window_desktop.hpp"

#include "../renderer/renderer.hpp"
#include "../vk/vk.hpp"
#include "../vk/vk_debug.hpp"
#include "../xengine/engine.hpp"
#include "./darwin/osx.h"

#ifdef X_SYSTEM_DESKTOP
X_BEGIN

static xWindowUpdateList WindowUpdateList;

bool WSIHasOpenWindow() {
	return !WindowUpdateList.IsEmpty();
}

static void window_close_callback(GLFWwindow * window) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	WindowPtr->Close();
}

static void window_refresh_callback(GLFWwindow * window) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	WindowPtr->OnRefresh();
}

static void window_resize_callback(struct GLFWwindow * window, int w, int h) {
	ssize32_t width     = (ssize32_t)w;
	ssize32_t height    = (ssize32_t)h;
	auto      WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	WindowPtr->OnResized(width, height);
}

static void window_content_scale_callback(GLFWwindow * window, float xscale, float yscale) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	WindowPtr->OnContentScaleUpdated(xscale, yscale);
}

static void window_cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	WindowPtr->OnCursorMove(xpos, ypos);
}

static void window_disabled_cursor_position_callback(GLFWwindow * window, double xpos, double ypos) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	WindowPtr->OnCursorMove(xpos, ypos);
	glfwSetCursorPos(window, 0, 0);
}

static void window_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	auto WindowPtr = (xDesktopWindow *)(glfwGetWindowUserPointer(window));
	(void)WindowPtr;
}

iWindow * CreateWindow(const xWindowSettings & Settings) {
	auto WindowPtr   = new xDesktopWindow;
	auto WindowGuard = xScopeGuard([=] { delete WindowPtr; });

	if (!WindowPtr->Init(Settings)) {
		return nullptr;
	}
	WindowPtr->OnCreated();
	WindowGuard.Dismiss();
	return WindowPtr;
}

void DestroyWindow(iWindow * WindowPtr) {
	WindowPtr->Clean();
	delete WindowPtr;
}

bool xDesktopWindow::Init(const xWindowSettings & Settings) {
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
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

	NativeHandle.Value = win;
	WindowUpdateList.GrabTail(*this);
	auto HandleGuard = xScopeGuard([&] {
		glfwDestroyWindow(win);
		xWindowUpdateList::Remove(*this);
		Reset(NativeHandle.Value);
	});

	FullScreen_ = Monitor;
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

	if (!CreateRenderSurface()) {
		return false;
	}

	HandleGuard.Dismiss();
	return true;
}

bool xDesktopWindow::CreateRenderSurface() {
	auto SurfaceHandle = (VkSurfaceKHR)VK_NULL_HANDLE;
	auto SurfaceResult = glfwCreateWindowSurface(VulkanInstance, NativeHandle.Value, nullptr, &SurfaceHandle);
	if (SurfaceResult != VK_SUCCESS) {
		return false;
	}
	auto SurfaceHandleGuard = xScopeGuard([&] { vkDestroySurfaceKHR(VulkanInstance, Steal(SurfaceHandle), nullptr); });
	if (!xRenderer::Spawn(std::move(SurfaceHandle))) {
		return false;
	}
	SurfaceHandleGuard.Dismiss();
	return true;
}

void xDesktopWindow::OnCreated() {
	auto win = NativeHandle.Value;
	int  w, h;
	glfwGetWindowSize(win, &w, &h);
	OnResized(w, h);
	float rx, ry;
	glfwGetWindowContentScale(win, &rx, &ry);
	OnContentScaleUpdated(rx, ry);
}

void xDesktopWindow::Close() {
	assert(NativeHandle.Value);
	xWindowUpdateList::Remove(*this);
	glfwDestroyWindow(Steal(NativeHandle.Value));
	OnClosed();
}

void xDesktopWindow::Clean() {
	if (!IsClosed()) {
		Close();
	}
}

bool xDesktopWindow::IsClosed() {
	return !NativeHandle.Value;
}

bool xDesktopWindow::IsFullScreen() {
	return FullScreen_;
}

void xDesktopWindow::SetCursorMode(eWindowCursorMode Mode) {
	auto win = NativeHandle.Value;
	switch (Mode) {
		case eWindowCursorMode::Native: {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(win, &window_cursor_position_callback);
		} break;

		case eWindowCursorMode::Hidden: {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetCursorPosCallback(win, &window_cursor_position_callback);
		} break;

		case eWindowCursorMode::Offset: {
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPosCallback(win, &window_disabled_cursor_position_callback);
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

void xDesktopWindow::Update() {
}

void xDesktopWindow::OnClosed() {
}

void WSILoopOnce() {
	glfwWaitEventsTimeout(0.05);
	WindowUpdateList.ForEach([](xWindowUpdateListNode & N) {
		auto & Window = static_cast<xDesktopWindow &>(N);
		Window.Update();
	});
	if (WindowUpdateList.IsEmpty()) {
		StopXEngine();
	}
}

X_END
#endif
