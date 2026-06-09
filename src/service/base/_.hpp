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

struct xServerIdComponent {
	uint32_t Id;
	uint16_t Random16;
	uint16_t Checksum;
};

struct xServerIdInternal {
	xServerType Type;
	uint32_t	ObjectId;
};

X_PRIVATE xServerIdComponent ExtractServerIdComponent(uint64_t ServerId);
X_PRIVATE xServerIdInternal	 ExtractServerIdInternalFromPureId(uint32_t Id);

X_API xServerType		ExtractServerType(uint64_t ServerId);
X_API uint32_t			ExtractServerObjectId(uint64_t ServerId);
X_API xServerIdInternal ExtractServerIdInternal(uint64_t ServerId);

X_SERVICE_END
