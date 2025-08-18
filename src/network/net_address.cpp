#include "./net_address.hpp"

#include "../core/string.hpp"

X_BEGIN

std::strong_ordering operator<=>(const xNetAddress & lhs, const xNetAddress & rhs) {
	if (lhs.Type != rhs.Type) {
		return lhs.Type <=> rhs.Type;
	}
	if (lhs.Type == xNetAddress::UNSPEC) {
		assert(!lhs.Port && !rhs.Port);
		return std::strong_ordering::equal;
	}
	if (lhs.Type == xNetAddress::IPV4) {
		auto diff = memcmp(lhs.SA4, rhs.SA4, 4);
		if (!diff) {
			return lhs.Port <=> rhs.Port;
		}
		return diff <=> 0;
	}
	if (lhs.Type == xNetAddress::IPV6) {
		auto diff = memcmp(lhs.SA6, rhs.SA6, 16);
		if (!diff) {
			return lhs.Port <=> rhs.Port;
		}
		return diff <=> 0;
	}
	Fatal("Invalid address type");
}

static xNetAddress InternalParse4(const char * IpStr, uint16_t Port) {
	auto Result = xNetAddress();
	auto parse4 = inet_pton(AF_INET, IpStr, Result.SA4);
	if (1 == parse4) {
		Result.Type = xNetAddress::IPV4;
		Result.Port = Port;
	}
	return Result;
}

static xNetAddress InternalParse6(const char * IpStr, uint16_t Port) {
	auto Result = xNetAddress();
	auto parse6 = inet_pton(AF_INET6, IpStr, Result.SA6);
	if (1 == parse6) {
		Result.Type = xNetAddress::IPV6;
		Result.Port = Port;
		return Result;
	}
	return Result;
}

xNetAddress xNetAddress::Parse(const std::string_view & AddressStr) {
	if (AddressStr.empty() || AddressStr.length() >= 48) {
		return {};
	}

	char     IpBuffer[48] = {};
	uint16_t Port         = 0;

	if (AddressStr.front() == '[') {
		if (AddressStr.back() == ']') {  // ipv6 w/o port
			memcpy(IpBuffer, AddressStr.data() + 1, AddressStr.size() - 2);
			return InternalParse6(IpBuffer, 0);
		}
		auto PortIndex = AddressStr.find_last_of(':');
		if (PortIndex == AddressStr.npos || AddressStr[PortIndex - 1] != ']') {  // invalid ipv6
			return {};
		}
		auto TestPort = MakeUnsigned(atoll(AddressStr.data() + PortIndex + 1));
		if (TestPort > 65535) {
			return {};
		}
		Port = (uint16_t)TestPort;
		memcpy(IpBuffer, AddressStr.data() + 1, PortIndex - 2);
		return InternalParse6(IpBuffer, Port);
	}

	// try ipv4 now:
	auto PortIndex = AddressStr.find_last_of(':');
	if (PortIndex == AddressStr.npos) {  // no port
		memcpy(IpBuffer, AddressStr.data(), AddressStr.size());
		return InternalParse4(IpBuffer, 0);
	}
	memcpy(IpBuffer, AddressStr.data(), PortIndex);

	auto TestPort = MakeUnsigned(atoll(AddressStr.data() + PortIndex + 1));
	if (TestPort > 65535) {
		return {};
	}
	Port = (uint16_t)TestPort;
	return InternalParse4(IpBuffer, Port);
}

xNetAddress xNetAddress::Parse(const sockaddr * SockAddrPtr) {
	auto Result = xNetAddress();
	if (SockAddrPtr->sa_family == AF_INET) {
		auto Addr4Ptr = (const sockaddr_in *)SockAddrPtr;
		Result.Type   = xNetAddress::IPV4;
		Result.Port   = ntohs(Addr4Ptr->sin_port);
		memcpy(Result.SA4, &Addr4Ptr->sin_addr, sizeof(Result.SA4));
	} else if (SockAddrPtr->sa_family == AF_INET6) {
		auto Addr6Ptr = (const sockaddr_in6 *)SockAddrPtr;
		Result.Type   = xNetAddress::IPV6;
		Result.Port   = ntohs(Addr6Ptr->sin6_port);
		memcpy(Result.SA6, &Addr6Ptr->sin6_addr, sizeof(Result.SA6));
	}
	return Result;
}

xNetAddress xNetAddress::Parse(const sockaddr_in * SockAddr4Ptr) {
	assert(SockAddr4Ptr->sin_family == AF_INET);
	auto Ret = Make4();
	memcpy(Ret.SA4, &SockAddr4Ptr->sin_addr, sizeof(Ret.SA4));
	Ret.Port = ntohs(SockAddr4Ptr->sin_port);
	return Ret;
}

xNetAddress xNetAddress::Parse(const sockaddr_in6 * SockAddr6Ptr) {
	assert(SockAddr6Ptr->sin6_family == AF_INET6);
	auto Ret = Make6();
	memcpy(Ret.SA6, &SockAddr6Ptr->sin6_addr, sizeof(Ret.SA6));
	Ret.Port = ntohs(SockAddr6Ptr->sin6_port);
	return Ret;
}

xNetAddress xNetAddress::Parse(const sockaddr_storage * SockAddrStoragePtr) {
	return Parse((const sockaddr *)SockAddrStoragePtr);
}

std::string xNetAddress::IpToString() const {
	char Buffer[64];
	if (Type == UNSPEC) {
		return "<unspecified>";
	}
	if (Type == IPV4) {
		return { Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d", (int)SA4[0], (int)SA4[1], (int)SA4[2], (int)SA4[3]) };
	}
	// ipv6
	return { Buffer,
			 (size_t)sprintf(
				 Buffer,
				 "%02x%02x:%02x%02x:"
				 "%02x%02x:%02x%02x:"
				 "%02x%02x:%02x%02x:"
				 "%02x%02x:%02x%02x",
				 (int)SA6[0], (int)SA6[1], (int)SA6[2], (int)SA6[3], (int)SA6[4], (int)SA6[5], (int)SA6[6], (int)SA6[7], (int)SA6[8], (int)SA6[9], (int)SA6[10], (int)SA6[11],
				 (int)SA6[12], (int)SA6[13], (int)SA6[14], (int)SA6[15]
			 ) };
}

std::string xNetAddress::ToString() const {
	char Buffer[64];
	if (Type == UNSPEC) {
		return { Buffer, (size_t)sprintf(Buffer, "<unspecified>:%u", (unsigned)Port) };
	}
	if (Type == IPV4) {
		return { Buffer, (size_t)sprintf(Buffer, "%d.%d.%d.%d:%u", (int)SA4[0], (int)SA4[1], (int)SA4[2], (int)SA4[3], (int)Port) };
	}
	// ipv6
	return { Buffer,
			 (size_t)sprintf(
				 Buffer,
				 "["
				 "%02x%02x:%02x%02x:"
				 "%02x%02x:%02x%02x:"
				 "%02x%02x:%02x%02x:"
				 "%02x%02x:%02x%02x]"
				 ":%u",
				 (int)SA6[0], (int)SA6[1], (int)SA6[2], (int)SA6[3], (int)SA6[4], (int)SA6[5], (int)SA6[6], (int)SA6[7], (int)SA6[8], (int)SA6[9], (int)SA6[10], (int)SA6[11],
				 (int)SA6[12], (int)SA6[13], (int)SA6[14], (int)SA6[15], (int)Port
			 ) };
}

X_END
