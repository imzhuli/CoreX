#pragma once
#include "../core/core_min.hpp"

#include <compare>

X_BEGIN

class xObjectId {
private:
	uint64_t						id = 0;
	static constexpr const uint64_t RefCountMask = uint64_t(1) << 63;

public:
	X_INLINE	  operator uint64_t() const { return id; }
	X_INLINE bool IsRefCounted() const { return (id & (uint64_t(1) << 63)) != 0; }
	X_INLINE bool IsValid() const { return id != 0; }
	X_INLINE bool IsNull() const { return id == 0; }

	X_INLINE bool operator==(const xObjectId & p_id) const { return id == p_id.id; }
	X_INLINE bool operator!=(const xObjectId & p_id) const { return id != p_id.id; }
	X_INLINE std::strong_ordering operator<=>(const xObjectId & p_id) const { return id <=> p_id.id; }

	X_INLINE xObjectId() {}
	X_INLINE explicit xObjectId(const uint64_t p_id) { id = p_id; }
	X_INLINE explicit xObjectId(const int64_t p_id) { id = p_id; }
	X_INLINE void operator=(uint64_t p_uint64) { id = p_uint64; }
};

X_END
