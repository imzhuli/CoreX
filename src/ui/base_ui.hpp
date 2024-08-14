#pragma once
#include "../core/core_min.hpp"

X_BEGIN

enum struct eUIEventSourceType : uint16_t {
	UNSPEC,
	KEYBOARD,
	MOUSE,
	JOYPAD,
	USER,
};

struct iUIEventSource {
	virtual eUIEventSourceType GetUIEventSourceType() { return eUIEventSourceType::UNSPEC; }
};

enum struct eUIEventType : uint16_t {
	UNSPEC,
	KEYBOARD,
	MOUSE,
	JOYPAD,
	USER,
};

struct xUIEvent {
	eUIEventType     Type;
	iUIEventSource * Source;
	xVariable        Value;
};

class xBaseUI {
public:
	enum eUIType : int {
		UIT_CANVAS    = 0,
		UIT_LABEL     = 1,
		UIT_BUTTON    = 2,
		UIT_CHECKBOX  = 3,
		UIT_LIST      = 4,
		UIT_COMBOLIST = 5,
		UIT_INPUT     = 5,
		UIT_TEXTAREA  = 6,
		UIT_USER      = 1024,
	};

	X_INLINE bool IsEnabled() const { return Status & UIS_ENABLED; }
	X_INLINE void SetEnabled(bool Enabled) { Status |= (Enabled ? UIS_ENABLED : UIS_NO_FLAG); }
	X_INLINE bool CanHaveFocus() const { return Status & UIS_CAN_HAVE_FOCUS; }
	X_INLINE void SetCanHaveFocus(bool Enabled) { Status |= (Enabled ? UIS_CAN_HAVE_FOCUS : UIS_NO_FLAG); }
	X_INLINE bool IsFocused() const { return Status & UIS_HAS_FOCUS; }
	X_INLINE void SetFocused(bool Enabled) { Status |= (Enabled ? UIS_HAS_FOCUS : UIS_NO_FLAG); }

private:
	enum eUIStatus : uint32_t {
		UIS_NO_FLAG        = 0x00,
		UIS_ENABLED        = 0x01,
		UIS_VISIBLE        = 0x02,
		UIS_CAN_HAVE_FOCUS = 0x04,
		UIS_HAS_FOCUS      = 0x08,
	};

	uint32_t Status = UIS_NO_FLAG;
};

X_END
