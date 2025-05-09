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

xNetAddress xNetAddress::Parse(const char * IpStr, uint16_t Port) {
	auto Result = xNetAddress();
	auto parse4 = inet_pton(AF_INET, IpStr, Result.SA4);
	if (1 == parse4) {
		Result.Type = xNetAddress::IPV4;
		Result.Port = Port;
		return Result;
	}
	auto parse6 = inet_pton(AF_INET6, IpStr, Result.SA6);
	if (1 == parse6) {
		Result.Type = xNetAddress::IPV6;
		Result.Port = Port;
		return Result;
	}
	return Result;
}

xNetAddress xNetAddress::Parse(const std::string & AddressStr) {
	if (AddressStr.empty() || AddressStr.length() >= 64) {
		return {};
	}
	auto PortIndex = AddressStr.find_last_of(':');
	if (PortIndex == AddressStr.npos) {  // no port, test ipv4 then ipv6
		return Parse(AddressStr.c_str(), 0);
	}

	if (AddressStr[0] == '[') {  // ipv6 with port
		if (AddressStr[PortIndex - 1] == ']') {
			char IpBuffer[64];
			auto IpLength = PortIndex - 2;
			memcpy(IpBuffer, AddressStr.c_str() + 1, IpLength);
			IpBuffer[IpLength] = '\0';

			auto Result = xNetAddress();
			if (1 == inet_pton(AF_INET6, IpBuffer, Result.SA6)) {
				Result.Type = xNetAddress::IPV6;
				Result.Port = (uint16_t)atoll(AddressStr.c_str() + PortIndex + 1);
				return Result;
			}
			return {};
		}
		return {};  // invalid address
	}
	for (size_t i = 1; i < 4; ++i) {
		if (AddressStr[i] == '.') {  // ipv4 with port
			char IpBuffer[64];
			memcpy(IpBuffer, AddressStr.c_str(), PortIndex);
			IpBuffer[PortIndex] = '\0';

			auto Result = xNetAddress();
			if (1 == inet_pton(AF_INET, IpBuffer, Result.SA4)) {
				Result.Type = xNetAddress::IPV4;
				Result.Port = (uint16_t)atoll(AddressStr.c_str() + PortIndex + 1);
				return Result;
			}
			return {};
		}
	}
	// ipv6 w/o port
	auto Result = xNetAddress();
	if (1 == inet_pton(AF_INET6, AddressStr.c_str(), Result.SA6)) {
		Result.Type = xNetAddress::IPV6;
		return Result;
	}
	return {};
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
