#pragma once
#include "../core/core_min.hpp"

X_BEGIN

enum struct eUIEventType : uint16_t {
	UNSPEC,
	GET_WINDOW_FOCUS,
	LOST_WINDOW_FOCUS,
	KEYBOARD_INPUT,
	MOUSE_BUTTON,
	JOYPAD_AXIS,
	JOYPAD_BUTTON,
	USER,
};

struct xUIEvent final {
	eUIEventType Type;
	xVariable    EventSource;
	xVariable    Value;
	xVariable    ValueEx;
};

class xUIBaseElement {
public:
	enum eType : int {
		UIT_UNSPEC = 0,
		UIT_CANVAS,
		UIT_LABEL,
		UIT_BUTTON,
		UIT_CHECKBOX,
		UIT_LIST,
		UIT_COMBOLIST,
		UIT_INPUT,
		UIT_TEXTAREA,
		UIT_USER = 1024,
	};

	X_INLINE eType GetType() const { return Type; }
	X_INLINE bool  IsEnabled() const { return Status & UIF_ENABLED; }
	X_INLINE void  SetEnabled(bool Enabled) { Status |= (Enabled ? UIF_ENABLED : UIF_NO_FLAG); }
	X_INLINE bool  CanHaveFocus() const { return Status & UIF_CAN_HAVE_FOCUS; }
	X_INLINE void  SetCanHaveFocus(bool Enabled) { Status |= (Enabled ? UIF_CAN_HAVE_FOCUS : UIF_NO_FLAG); }
	X_INLINE bool  IsFocused() const { return Status & UIF_HAS_FOCUS; }
	X_INLINE void  SetFocused(bool Enabled) { Status |= (Enabled ? UIF_HAS_FOCUS : UIF_NO_FLAG); }

private:
	enum eUIFlag : uint32_t {
		UIF_NO_FLAG        = 0x00,
		UIF_ENABLED        = 0x01,
		UIF_VISIBLE        = 0x02,
		UIF_CAN_HAVE_FOCUS = 0x04,
		UIF_HAS_FOCUS      = 0x08,
	};

	eType    Type   = UIT_UNSPEC;
	uint32_t Status = UIF_NO_FLAG;
};

X_END
