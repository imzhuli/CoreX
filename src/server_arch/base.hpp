#pragma once
#include "../core/core_min.hpp"
#include "../network/net_address.hpp"

X_BEGIN

using xServerGroupId    = uint16_t;
using xServerInstanceId = uint16_t;
using xServerType       = uint32_t;
using xServerWeight     = uint32_t;

constexpr const xServerGroupId    SERVER_GROUP_UNSPEC    = 0;
constexpr const xServerInstanceId SERVER_INSTANCE_UNSPEC = 0;
constexpr const xServerType       SERVER_TYPE_UNSPEC     = 0;
constexpr const xServerWeight     SERVER_WEIGHT_UNSPEC   = 0;

struct xServerId {
	xServerGroupId    GroupId;
	xServerInstanceId InstanceId;

	X_INLINE constexpr xServerId() = default;
	X_INLINE constexpr xServerId(xServerInstanceId IID)
		: GroupId(0), InstanceId(IID) {
	}
	X_INLINE constexpr xServerId(xServerGroupId GID, xServerInstanceId IID)
		: GroupId(GID), InstanceId(IID) {
	}
	X_INLINE constexpr xServerId(const xServerId &) = default;

	X_INLINE constexpr operator uint32_t() const {
		return Value();
	}
	X_INLINE constexpr uint32_t Value() const {
		return ((uint32_t)GroupId << 16) | (uint32_t)InstanceId;
	}
};

struct xServerInfo {
	xServerId     Id      = {};
	xServerType   Type    = 0;
	xServerWeight Weight  = 0;
	xNetAddress   Address = {};
};

X_END
