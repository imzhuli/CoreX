#include "./net_address.hpp"

#include "../core/string.hpp"

X_BEGIN

std::strong_ordering operator<=>(const xNetAddress & lhs, const xNetAddress & rhs) {
	if (lhs.Type != rhs.Type) {
		return lhs.Type <=> rhs.Type;
	}
	if (lhs.Type == xNetAddress::UNSPEC) {
		return lhs.Port <=> rhs.Port;
	}
	if (lhs.Type == xNetAddress::IPV4) {
		auto diff = memcmp(lhs.Addr4, rhs.Addr4, 4);
		if (!diff) {
			return lhs.Port <=> rhs.Port;
		}
		return diff <=> 0;
	}
	if (lhs.Type == xNetAddress::IPV6) {
		auto diff = memcmp(lhs.Addr6, rhs.Addr6, 16);
		if (!diff) {
			return lhs.Port <=> rhs.Port;
		}
		return diff <=> 0;
	}
	Fatal("Invalid address type");
}

xNetAddress xNetAddress::Parse(const char * IpStr, uint16_t Port) {
	auto Result = xNetAddress();
	auto parse4 = inet_pton(AF_INET, IpStr, Result.Addr4);
	if (1 == parse4) {
		Result.Type = xNetAddress::IPV4;
		Result.Port = Port;
		return Result;
	}
	auto parse6 = inet_pton(AF_INET6, IpStr, Result.Addr6);
	if (1 == parse6) {
		Result.Type = xNetAddress::IPV6;
		Result.Port = Port;
		return Result;
	}
	return Result;
}

xNetAddress xNetAddress::Parse(const std::string & AddressStr) {
	uint16_t Port = 0;
	auto     Segs = Split(AddressStr, ":");
	if (Segs.size() > 2) {
		return {};
	}
	if (Segs.size() == 2) {
		Port = (uint16_t)atoll(Segs[1].c_str());
	}
	auto IpStr = Segs[0].c_str();
	return Parse(IpStr, Port);
}

xNetAddress xNetAddress::Parse(const sockaddr * SockAddrPtr) {
	auto Result = xNetAddress();
	if (SockAddrPtr->sa_family == AF_INET) {
		auto Addr4Ptr = (const sockaddr_in *)SockAddrPtr;
		Result.Type   = xNetAddress::IPV4;
		Result.Port   = ntohs(Addr4Ptr->sin_port);
		memcpy(Result.Addr4, &Addr4Ptr->sin_addr, sizeof(Result.Addr4));
	} else if (SockAddrPtr->sa_family == AF_INET6) {
		auto Addr6Ptr = (const sockaddr_in6 *)SockAddrPtr;
		Result.Type   = xNetAddress::IPV6;
		Result.Port   = ntohs(Addr6Ptr->sin6_port);
		memcpy(Result.Addr6, &Addr6Ptr->sin6_addr, sizeof(Result.Addr6));
	}
	return Result;
}

xNetAddress xNetAddress::Parse(const sockaddr_in * SockAddr4Ptr) {
	assert(SockAddr4Ptr->sin_family == AF_INET);
	auto Ret = Make4();
	memcpy(Ret.Addr4, &SockAddr4Ptr->sin_addr, sizeof(Ret.Addr4));
	Ret.Port = ntohs(SockAddr4Ptr->sin_port);
	return Ret;
}

xNetAddress xNetAddress::Parse(const sockaddr_in6 * SockAddr6Ptr) {
	assert(SockAddr6Ptr->sin6_family == AF_INET6);
	auto Ret = Make6();
	memcpy(Ret.Addr6, &SockAddr6Ptr->sin6_addr, sizeof(Ret.Addr6));
	Ret.Port = ntohs(SockAddr6Ptr->sin6_port);
	return Ret;
}

xNetAddress xNetAddress::Parse(const sockaddr_storage * SockAddrStoragePtr) {
	return Parse((const sockaddr *)SockAddrStoragePtr);
}

std::string xNetAddress::IpToString() const {
	char Buffer[64];
	if (Type == UNSPEC) {
		return "unknown";
	}
	if (Type == IPV4) {
		return { Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d", (int)Addr4[0], (int)Addr4[1], (int)Addr4[2], (int)Addr4[3]) };
	}
	// ipv6
	return { Buffer,
			 (size_t)sprintf(
				 Buffer,
				 "%02x:%02x:%02x:%02x:"
				 "%02x:%02x:%02x:%02x:"
				 "%02x:%02x:%02x:%02x:"
				 "%02x:%02x:%02x:%02x",
				 (int)Addr6[0], (int)Addr6[1], (int)Addr6[2], (int)Addr6[3], (int)Addr6[4], (int)Addr6[5], (int)Addr6[6], (int)Addr6[7], (int)Addr6[8], (int)Addr6[9],
				 (int)Addr6[10], (int)Addr6[11], (int)Addr6[12], (int)Addr6[13], (int)Addr6[14], (int)Addr6[15]
			 ) };
}

std::string xNetAddress::ToString() const {
	char Buffer[64];
	if (Type == UNSPEC) {
		return "unknown";
	}
	if (Type == IPV4) {
		return { Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d:%u", (int)Addr4[0], (int)Addr4[1], (int)Addr4[2], (int)Addr4[3], (int)Port) };
	}
	// ipv6
	return { Buffer,
			 (size_t)sprintf(
				 Buffer,
				 "%02x:%02x:%02x:%02x:"
				 "%02x:%02x:%02x:%02x:"
				 "%02x:%02x:%02x:%02x:"
				 "%02x:%02x:%02x:%02x:"
				 "%u",
				 (int)Addr6[0], (int)Addr6[1], (int)Addr6[2], (int)Addr6[3], (int)Addr6[4], (int)Addr6[5], (int)Addr6[6], (int)Addr6[7], (int)Addr6[8], (int)Addr6[9],
				 (int)Addr6[10], (int)Addr6[11], (int)Addr6[12], (int)Addr6[13], (int)Addr6[14], (int)Addr6[15], (int)Port
			 ) };
}

X_END
