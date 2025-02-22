#pragma once
#include "./base.hpp"

#include <array>
#include <compare>
#include <string>

X_BEGIN

struct xNetAddress final {
	enum eType : uint16_t { UNSPEC, IPV4, IPV6 };

	eType Type = UNSPEC;
	union {
		ubyte SA4[4];
		ubyte SA6[16];
		ubyte __HOLDER__[16] = {};  // used for zero init
	};
	uint16_t Port = 0;

	// asserts
	static_assert(sizeof(SA4) <= sizeof(__HOLDER__));
	static_assert(sizeof(SA6) <= sizeof(__HOLDER__));

	// methods:
	X_INLINE bool     IsV4() const { return Type == IPV4; }
	X_INLINE bool     IsV6() const { return Type == IPV6; }
	X_INLINE explicit operator bool() const { return Type != UNSPEC; }

	using xKeyType = std::array<ubyte, 20>;
	X_INLINE xKeyType AsKey() const {
		static_assert(sizeof(*this) == std::tuple_size_v<xKeyType>);
		xKeyType Ret;
		memcpy(Ret.data(), this, sizeof(*this));
		return Ret;
	}

	X_INLINE int GetAddressFamily() const {
		if (Type == IPV4) {
			return AF_INET;
		}
		if (Type == IPV6) {
			return AF_INET6;
		}
		return AF_UNSPEC;
	}

	X_INLINE xNetAddress RemovePort() const {
		auto A = *this;
		A.Port = 0;
		return A;
	}
	X_INLINE xNetAddress Decay() const { return { .Type = this->Type }; }

	X_INLINE void Dump(sockaddr_in * Addr4Ptr) const {
		assert(IsV4());
		memset(Addr4Ptr, 0, sizeof(*Addr4Ptr));
		auto & Addr4     = *Addr4Ptr;
		Addr4.sin_family = AF_INET;
		Addr4.sin_addr   = reinterpret_cast<const decltype(sockaddr_in::sin_addr) &>(SA4);
		Addr4.sin_port   = htons(Port);
	}

	X_INLINE void Dump(sockaddr_in6 * Addr6Ptr) const {
		assert(IsV6());
		memset(Addr6Ptr, 0, sizeof(*Addr6Ptr));
		auto & Addr6      = *Addr6Ptr;
		Addr6.sin6_family = AF_INET6;
		Addr6.sin6_addr   = reinterpret_cast<const decltype(sockaddr_in6::sin6_addr) &>(SA6);
		Addr6.sin6_port   = htons(Port);
	}

	X_INLINE size_t Dump(sockaddr_storage * AddrStoragePtr) const {
		if (IsV4()) {
			Dump((sockaddr_in *)AddrStoragePtr);
			return sizeof(sockaddr_in);
		}
		if (IsV6()) {
			Dump((sockaddr_in6 *)AddrStoragePtr);
			return sizeof(sockaddr_in6);
		}
		X_PFATAL("invalid address type");
		*AddrStoragePtr           = sockaddr_storage{};
		AddrStoragePtr->ss_family = AF_UNSPEC;
		return 0;
	}

	X_API_MEMBER std::string IpToString() const;
	X_API_MEMBER std::string ToString() const;

	X_STATIC_INLINE xNetAddress Make4() { return xNetAddress{ .Type = IPV4 }; }
	X_STATIC_INLINE xNetAddress Make6() { return xNetAddress{ .Type = IPV6 }; }

	X_STATIC_INLINE xNetAddress Make4Raw(const void * RawPtr, uint16_t Port) {
		auto Address = xNetAddress{ .Type = IPV4, .Port = Port };
		memcpy(Address.SA4, RawPtr, sizeof(Address.SA4));
		return Address;
	}
	X_STATIC_INLINE xNetAddress Make6Raw(const void * RawPtr, uint16_t Port) {
		auto Address = xNetAddress{ .Type = IPV6, .Port = Port };
		memcpy(Address.SA6, RawPtr, sizeof(Address.SA6));
		return Address;
	}

	X_API_STATIC_MEMBER xNetAddress Parse(const char * IpStr, uint16_t Port);
	X_API_STATIC_MEMBER xNetAddress Parse(const std::string & AddressStr);

	X_API_STATIC_MEMBER xNetAddress Parse(const sockaddr * SockAddrPtr);
	X_API_STATIC_MEMBER xNetAddress Parse(const sockaddr_in * SockAddr4Ptr);
	X_API_STATIC_MEMBER xNetAddress Parse(const sockaddr_in6 * SockAddr6Ptr);
	X_API_STATIC_MEMBER xNetAddress Parse(const sockaddr_storage * SockAddrStoragePtr);
};

X_API std::strong_ordering operator<=>(const xNetAddress & lhs, const xNetAddress & rhs);

X_INLINE bool operator==(const xNetAddress & lhs, const xNetAddress & rhs) {
	return 0 == (lhs <=> rhs);
}

X_END
