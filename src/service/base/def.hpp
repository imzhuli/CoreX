#pragma once
#include "../../core/core_min.hpp"
#include "../../core/functional.hpp"
#include "../../network/net_address.hpp"
#include "../../network/packet.hpp"

// clang-format off
#define X_SERVICE_BEGIN X_BEGIN namespace service {
#define X_SERVICE_END }}
// clang-format on

X_SERVICE_BEGIN

using xServerType = uint8_t;
using xServerId	  = uint64_t;

struct xServerIdInternal {
	xServerType Type;
	uint32_t	ObjectId;
};

X_INLINE xServerType ExtractServerType(uint64_t ServerId) {
	assert(ServerId);
	return (xServerType)(ServerId >> 51);
}

X_INLINE xServerIdInternal ExtractServerIdInternal(uint64_t ServerId) {
	auto Id = uint32_t(ServerId >> 32);
	return {
		.Type	  = (xServerType)(Id >> 19),
		.ObjectId = Id & 0x07FFFF,
	};
}

X_SERVICE_END
