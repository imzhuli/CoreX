#include "./indexed_storage.hpp"

#include "./core_time.hpp"

X_BEGIN

static_assert(sizeof(xIndexId) == sizeof(uint64_t));

uint32_t xIndexId::TimeSeed() {
	return static_cast<uint32_t>(GetTimestampUS());
}

X_END
