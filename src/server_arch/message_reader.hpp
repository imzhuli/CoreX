#pragma once
#include "../core/core_min.hpp"
#include "../core/core_stream.hpp"
#include "../network/net_address.hpp"

#include <limits>

X_BEGIN

class xBinaryMessageReader {
public:
	X_INLINE void Reset(const void * Src, size_t SrcSize) {
		_Reader.Reset(Src);
		_RemainSize = SrcSize;
	}

	X_INLINE void   SetError() { _RemainSize = -1; }
	X_INLINE bool   HasError() const { return _RemainSize == -1; }
	X_INLINE size_t GetConsumedSize() const {
		if (HasError()) {
			return 0;
		}
		return _Reader.Offset();
	}

	X_INLINE const ubyte * GetCurrentPosition() {
		if (HasError()) {
			return nullptr;
		}
		return _Reader;
	}

	template <typename T, typename... tArgs>
	X_INLINE void R(T && Arg0, tArgs &&... Args) {
		_R(std::forward<T>(Arg0));
		R(std::forward<tArgs>(Args)...);
	}
	X_INLINE void R() {};

private:
	X_INLINE void _R(std::string & V) { V = _RB(); }
	X_INLINE void _R(std::string_view & V) { V = _ReadRawBlockView(); }
	X_INLINE void _R(xNetAddress & Addr) { _RAddr(Addr); }
	X_INLINE void _R(bool & V) { V = static_cast<bool>(_R1()); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 1 == sizeof(T)> _R(T & V) {
		V = static_cast<T>(_R1());
	}
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 2 == sizeof(T)> _R(T & V) {
		V = static_cast<T>(_R2());
	}
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 4 == sizeof(T)> _R(T & V) {
		V = static_cast<T>(_R4());
	}
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 8 == sizeof(T)> _R(T & V) {
		V = static_cast<T>(_R8());
	}

private:
	X_INLINE uint8_t _R1() {
		auto RequiredSize = (ssize_t)sizeof(uint8_t);
		if (_RemainSize < RequiredSize) {
			SetError();
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R1L();
	}
	X_INLINE uint16_t _R2() {
		auto RequiredSize = (ssize_t)sizeof(uint16_t);
		if (_RemainSize < RequiredSize) {
			SetError();
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R2L();
	}
	X_INLINE uint32_t _R4() {
		auto RequiredSize = (ssize_t)sizeof(uint32_t);
		if (_RemainSize < RequiredSize) {
			SetError();
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R4L();
	}
	X_INLINE uint64_t _R8() {
		auto RequiredSize = (ssize_t)sizeof(uint64_t);
		if (_RemainSize < RequiredSize) {
			SetError();
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R8L();
	}
	X_INLINE std::string _RB() {
		auto Size = _R4();
		if (_RemainSize < 0) {
			return {};
		}
		auto R = std::string();
		R.resize(Size);
		_ReadRawBlock(R.data(), Size);
		if (_RemainSize < 0) {
			return {};
		}
		return R;
	}

	X_INLINE std::string_view _ReadRawBlockView() {
		auto Size = _R4();
		if (_RemainSize < 0) {
			return {};
		}
		auto RequiredSize = static_cast<ssize_t>(Size);
		assert(RequiredSize < std::numeric_limits<ssize32_t>::max());
		if (_RemainSize < RequiredSize) {
			SetError();
			return {};
		}
		_RemainSize -= RequiredSize;
		return { (const char *)_Reader.Skip(Size), Size };
	}

	X_INLINE void _ReadRawBlock(void * Block, size_t Size) {
		auto RequiredSize = static_cast<ssize_t>(Size);
		assert(RequiredSize < std::numeric_limits<ssize32_t>::max());
		if (_RemainSize < RequiredSize) {
			SetError();
			return;
		}
		_RemainSize -= RequiredSize;
		_Reader.R(Block, Size);
	}
	void _RAddr(xNetAddress & Addr) {
		if (_RemainSize < 1) {
			Addr = xNetAddress();
			SetError();
			return;
		}
		auto Type = _Reader.R1();
		if (Type == 0x00) {
			Addr         = xNetAddress();
			_RemainSize -= 1;
			return;
		}
		if (Type == 0x04) {
			auto RequiredSize = decltype(_RemainSize)(7);
			if (_RemainSize < RequiredSize) {
				Addr = xNetAddress();
				SetError();
				return;
			}
			Addr = xNetAddress::Make4();
			_Reader.R(Addr.SA4, 4);
			Addr.Port    = _Reader.R2L();
			_RemainSize -= RequiredSize;
			return;
		}
		if (Type == 0x06) {
			auto RequiredSize = decltype(_RemainSize)(19);
			if (_RemainSize < RequiredSize) {
				Addr         = xNetAddress();
				_RemainSize -= 1;
				return;
			}
			Addr = xNetAddress::Make6();
			_Reader.R(Addr.SA6, 16);
			Addr.Port    = _Reader.R2L();
			_RemainSize -= RequiredSize;
			return;
		}
		// error:
		Addr = xNetAddress();
		SetError();
		return;
	}

private:
	xStreamReader _Reader;
	ssize_t       _RemainSize = 0;
};

X_END
