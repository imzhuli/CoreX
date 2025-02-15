#pragma once
#include "../core/core_min.hpp"
#include "../core/core_stream.hpp"
#include "../network/net_address.hpp"

#include <limits>

X_BEGIN

class xBinaryMessageWriter {
public:
	X_INLINE void Reset(void * Dest, size_t DestSize) {
		_Writer.Reset(Dest);
		_RemainSize = static_cast<ssize_t>(DestSize);
	}

	X_INLINE void    SetError() { _RemainSize = -1; }
	X_INLINE bool    HasError() const { return _RemainSize < 0; }
	X_INLINE ubyte * GetCurrentPosition() const {
		if (HasError()) {
			return nullptr;
		}
		return _Writer;
	}
	X_INLINE size_t GetConsumedSize() const {
		if (HasError()) {
			return 0;
		}
		return _Writer.Offset();
	}
	X_INLINE bool HasRemainSize(size_t Size) const {
		if (HasError()) {
			return false;
		}
		return static_cast<size_t>(_RemainSize) <= Size;
	}

	template <typename T, typename... tArgs>
	X_INLINE void W(T && Arg0, tArgs &&... Args) {
		_W(std::forward<T>(Arg0));
		W(std::forward<tArgs>(Args)...);
	}
	X_INLINE void W() {};

private:
	// W
	X_INLINE void _W(const xNetAddress & Address) { _WAddr(Address); }
	X_INLINE void _W(const std::string_view & V) { _WB(V.data(), V.size()); }

	template <typename T>
	X_INLINE std::enable_if_t<std::is_same_v<bool, std::remove_cvref_t<T>> && !std::is_rvalue_reference_v<T &&>> _W(T && V) {
		_W1(std::forward<T>(V) ? (uint8_t)1 : (uint8_t)0);
	}
	template <typename T>
	X_INLINE
		std::enable_if_t<!std::is_same_v<bool, std::remove_cvref_t<T>> && std::is_integral_v<std::remove_cvref_t<T>> && !std::is_rvalue_reference_v<T &&> && 1 == sizeof(T)>
		_W(T && V) {
		_W1(V);
	}
	template <typename T>
	X_INLINE
		std::enable_if_t<!std::is_same_v<bool, std::remove_cvref_t<T>> && std::is_integral_v<std::remove_cvref_t<T>> && !std::is_rvalue_reference_v<T &&> && 2 == sizeof(T)>
		_W(T && V) {
		_W2(V);
	}
	template <typename T>
	X_INLINE
		std::enable_if_t<!std::is_same_v<bool, std::remove_cvref_t<T>> && std::is_integral_v<std::remove_cvref_t<T>> && !std::is_rvalue_reference_v<T &&> && 4 == sizeof(T)>
		_W(T && V) {
		_W4(V);
	}
	template <typename T>
	X_INLINE
		std::enable_if_t<!std::is_same_v<bool, std::remove_cvref_t<T>> && std::is_integral_v<std::remove_cvref_t<T>> && !std::is_rvalue_reference_v<T &&> && 8 == sizeof(T)>
		_W(T && V) {
		_W8(V);
	}

private:
	X_INLINE void _W1(uint8_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			SetError();
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W1L(V);
	}
	X_INLINE void _W2(uint16_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			SetError();
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W2L(V);
	}
	X_INLINE void _W4(uint32_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			SetError();
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W4L(V);
	}
	X_INLINE void _W8(uint64_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			SetError();
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W8L(V);
	}
	X_INLINE void _WB(const void * Block, size_t Size) {
		_W4((ssize32_t)Size);
		_WriteRawBlock(Block, Size);
	}
	X_INLINE void _WriteRawBlock(const void * Block, size_t Size) {
		auto RequiredSize = static_cast<ssize_t>(Size);
		assert(RequiredSize < std::numeric_limits<ssize32_t>::max());
		if (_RemainSize < RequiredSize) {
			SetError();
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W(Block, Size);
	}
	X_INLINE void _WAddr(const xNetAddress & Addr) {
		if (!Addr) {
			if (_RemainSize < 1) {
				SetError();
				return;
			}
			_Writer.W1(0);
			_RemainSize -= 1;
			return;
		}
		if (Addr.IsV4()) {
			auto RequiredSize = decltype(_RemainSize)(7);
			if (_RemainSize < RequiredSize) {
				SetError();
				return;
			}
			_Writer.W1(0x04);
			_Writer.W(Addr.SA4, 4);
			_Writer.W2L(Addr.Port);
			_RemainSize -= RequiredSize;
			return;
		}
		if (Addr.IsV6()) {
			auto RequiredSize = decltype(_RemainSize)(19);
			if (_RemainSize < RequiredSize) {
				SetError();
				return;
			}
			_Writer.W1(0x06);
			_Writer.W(Addr.SA6, 16);
			_Writer.W2L(Addr.Port);
			_RemainSize -= RequiredSize;
			return;
		}
		X_PFATAL("Invalid address type");
	}

private:
	xStreamWriter _Writer;
	ssize_t       _RemainSize = 0;
};

X_END
