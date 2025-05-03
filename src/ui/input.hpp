#pragma once
#include "./base_ui.hpp"

#include <bitset>

X_BEGIN

enum eKeyCode : uint_fast16_t {

};

class xInputManager {
public:
	bool Init();
	void Clean();

	// 接受来自环境的原始输入事件, 并转化成潜在的输入事件
	// 例如: 当窗口切走再切回时, 可能按键状态已经改变, 但系统并未给出按键变化事件, 此时需要检测所有按键的变化, 并生产输入事件
	void AcceptRawInputEvent(const xUIEvent & Event);
	void DetectRawInputChanges();

	// 向查询者(事件循环), 输出转化后的事件
	const xUIEvent * GetInputEvent();

protected:
	X_INLINE bool       IsEventQueueEmpty() const { return EventStartIndex == EventEndIndex; }
	X_INLINE bool       IsEventQueueFull() const { return EventStartIndex == (0x0FFu & (EventEndIndex + 1)); }
	X_INLINE xUIEvent * PeekLastEvent() {
		if (RequireRawEventDetection && !IsEventQueueFull()) {
			DetectRawInputChanges();
		}
		if (IsEventQueueEmpty()) {
			return nullptr;
		}
		return &EventQueue[EventStartIndex];
	}

private:
	xUIEvent EventQueue[256]          = {};
	size_t   EventStartIndex          = 0;
	size_t   EventEndIndex            = 0;
	bool     RequireRawEventDetection = false;

	std::bitset<128> KeyboardAndMouseButtonState = {};

	static_assert(IsPow2(std::extent_v<decltype(EventQueue)>));
};

X_END
