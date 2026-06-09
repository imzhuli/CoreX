#pragma once
#include "../../core/core_min.hpp"
#include "../../network/packet.hpp"

X_BEGIN

namespace service {

	using xServerType = uint8_t;
	using xServerId	  = uint64_t;

}  // namespace service

// clang-format off
#define X_SERVICE_BEGIN X_BEGIN namespace service {
#define X_SERVICE_END }}

X_END
