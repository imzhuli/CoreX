#pragma once
#include "../core/core_min.hpp"
#include "../network/net_address.hpp"
#include "../network/packet.hpp"

#include <limits>

X_BEGIN

class xBinaryMessage {

public:
	X_API_MEMBER size_t Serialize(void * Dst, size_t Size);
	X_API_MEMBER size_t Deserialize(const void * Src, size_t Size);

protected:
	X_API_MEMBER virtual void SerializeMembers();
	X_API_MEMBER virtual void DeserializeMembers();

protected:  // clang-format off
	template <typename T, typename... tArgs>
	X_INLINE void W(T && Arg0, tArgs &&... Args) {
		_W(std::forward<T>(Arg0)); W(std::forward<tArgs>(Args)...);
	}
	X_INLINE void W(){};

	template <typename T, typename... tArgs>
	X_INLINE void R(T && Arg0, tArgs &&... Args) {
		_R(std::forward<T>(Arg0)); R(std::forward<tArgs>(Args)...);
	}
	X_INLINE void R(){};

private:
	// W
	X_INLINE void _W(const std::string_view & V) { _WB(V.data(), V.size()); }
	X_INLINE void _W(const xNetAddress & Addr) { _W(Addr.ToString()); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<std::remove_reference_t<T &&>> && !std::is_rvalue_reference_v<T &&> && 1 == sizeof(T)> _W(T && V) { _W1(V); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<std::remove_reference_t<T &&>> && !std::is_rvalue_reference_v<T &&> && 2 == sizeof(T)> _W(T && V) { _W2(V); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<std::remove_reference_t<T &&>> && !std::is_rvalue_reference_v<T &&> && 4 == sizeof(T)> _W(T && V) { _W4(V); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<std::remove_reference_t<T &&>> && !std::is_rvalue_reference_v<T &&> && 8 == sizeof(T)> _W(T && V) { _W8(V); }

	// R
	X_INLINE void _R(std::string & V) { V = _RB(); }
	X_INLINE void _R(xNetAddress & Addr) { std::string AddrString; _R(AddrString); Addr = xNetAddress::Parse(AddrString); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 1 == sizeof(T)> _R(T & V) { V = static_cast<T>(_R1()); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 2 == sizeof(T)> _R(T & V) { V = static_cast<T>(_R2()); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 4 == sizeof(T)> _R(T & V) { V = static_cast<T>(_R4()); }
	template <typename T>
	X_INLINE std::enable_if_t<std::is_integral_v<T> && 8 == sizeof(T)> _R(T & V) { V = static_cast<T>(_R8()); }

private:  // clang-format on
	/* write */
	X_INLINE void _W1(uint8_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W1L(V);
	}
	X_INLINE void _W2(uint16_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W2L(V);
	}
	X_INLINE void _W4(uint32_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W4L(V);
	}
	X_INLINE void _W8(uint64_t V) {
		auto RequiredSize = (ssize_t)sizeof(V);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W8L(V);
	}
	X_INLINE void _WB(const void * Block, size_t Size) {
		_W4((ssize32_t)Size);
		if (_RemainSize < 0) {
			return;
		}
		_WriteRawBlock(Block, Size);
	}
	X_INLINE void _WriteRawBlock(const void * Block, size_t Size) {
		auto RequiredSize = static_cast<ssize_t>(Size);
		assert(RequiredSize < std::numeric_limits<ssize32_t>::max());
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return;
		}
		_RemainSize -= RequiredSize;
		_Writer.W(Block, Size);
	}

	/* read */
	X_INLINE uint8_t _R1() {
		auto RequiredSize = (ssize_t)sizeof(uint8_t);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R1L();
	}
	X_INLINE uint16_t _R2() {
		auto RequiredSize = (ssize_t)sizeof(uint16_t);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R2L();
	}
	X_INLINE uint32_t _R4() {
		auto RequiredSize = (ssize_t)sizeof(uint32_t);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return 0;
		}
		_RemainSize -= RequiredSize;
		return _Reader.R4L();
	}
	X_INLINE uint64_t _R8() {
		auto RequiredSize = (ssize_t)sizeof(uint64_t);
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
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
	X_INLINE void _ReadRawBlock(void * Block, size_t Size) {
		auto RequiredSize = static_cast<ssize_t>(Size);
		assert(RequiredSize < std::numeric_limits<ssize32_t>::max());
		if (_RemainSize < RequiredSize) {
			_RemainSize = -1;
			return;
		}
		_RemainSize -= RequiredSize;
		_Reader.R(Block, Size);
	}

private:
	xStreamWriter _Writer;
	xStreamReader _Reader;
	ssize_t       _RemainSize = 0;
};

X_API size_t WritePacket(xPacketCommandId CmdId, xPacketRequestId RequestId, void * Buffer, size_t BufferSize, xBinaryMessage & Message);

X_END
