#pragma once

#include "../core/core_min.hpp"

#include <compare>
X_BEGIN

class xObjectBase;
class xObjectIdManager;
class xObjectIdManagerMini;

class xObjectBase : xNonCopyable {
public:
	static constexpr const uint32_t MAX_OBJECT_ID = 0x0FFF'FFFFu;

public:
	X_INLINE void InitId(uint32_t NewId) {
		assert(!Id);
		Id = NewId;
	}
	X_INLINE uint32_t GetId() const { return Id; }

	X_INLINE void RetainRef() {
		++RefCount;
		assert(RefCount);
	}
	X_INLINE uint32_t ReleaseRef() { return --RefCount; }

	X_INLINE bool operator==(const xObjectBase & O) const { return Id == O.Id; }
	X_INLINE std::strong_ordering operator<=>(const xObjectBase & O) const { return Id <=> O.Id; }

private:
	uint32_t Id       = 0;
	uint32_t RefCount = 0;
};

class xObjectIdManager final : xNonCopyable {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();
	X_API_MEMBER void Reset();

	X_API_MEMBER uint32_t Acquire();
	X_API_MEMBER bool     Acquire(uint32_t Id);
	X_API_MEMBER void     Release(uint32_t Id);
	X_API_MEMBER bool     IsInUse(uint32_t Id) const;

private:
	uint64_t * Bitmap = nullptr;

	static constexpr const uint64_t BASE_ONE  = 1;
	static constexpr const uint64_t L0_Start  = 0;
	static constexpr const uint64_t L0_Size   = 1;
	static constexpr const uint64_t L1_Start  = L0_Start + L0_Size;
	static constexpr const uint64_t L1_Size   = 64;
	static constexpr const uint64_t L2_Start  = L1_Start + L1_Size;
	static constexpr const uint64_t L2_Size   = L1_Size * 64;
	static constexpr const size_t   AllocSize = sizeof(uint64_t) * (L0_Size + L1_Size + L2_Size);

public:
	static constexpr const size_t MaxObjectId = L2_Size * 64;
	static_assert(MaxObjectId <= xObjectBase::MAX_OBJECT_ID);
};

class xObjectIdManagerMini final : xNonCopyable {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();
	X_API_MEMBER void Reset();

	X_API_MEMBER uint32_t Acquire();
	X_API_MEMBER bool     Acquire(uint32_t Id);
	X_API_MEMBER void     Release(uint32_t Id);
	X_API_MEMBER bool     IsInUse(uint32_t Id) const;

private:
	uint64_t * Bitmap = nullptr;

	static constexpr const uint64_t BASE_ONE  = 1;
	static constexpr const uint64_t L0_Start  = 0;
	static constexpr const uint64_t L0_Size   = 1;
	static constexpr const uint64_t L1_Start  = L0_Start + L0_Size;
	static constexpr const uint64_t L1_Size   = 64;
	static constexpr const size_t   AllocSize = sizeof(uint64_t) * (L0_Size + L1_Size);

public:
	static constexpr const size_t MaxObjectId = L1_Size * 64;
	static_assert(MaxObjectId <= xObjectBase::MAX_OBJECT_ID);
};

X_END
