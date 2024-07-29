#pragma once

#include "../core/core_min.hpp"

#include <compare>
X_BEGIN

// clang-format off
class xObjectBase {
private:
	uint64_t						Id = 0;
	static constexpr const uint64_t OBJECT_ID_MASK        = 0x00FFFFFFULL;
	static constexpr const uint64_t ENABLE_REF_COUNT_MASK = uint64_t(1) << 24;

public:
	X_INLINE	  operator uint64_t() const { return Id; }
	X_INLINE bool IsRefCounted() const { return Id & ENABLE_REF_COUNT_MASK; }
	X_INLINE bool IsValid() const { return Id != 0; }
	X_INLINE bool IsNull() const { return Id == 0; }

	X_INLINE bool operator==(const xObjectBase & O) const { return Id == O.Id; }
	X_INLINE bool operator!=(const xObjectBase & O) const { return Id != O.Id; }
	X_INLINE std::strong_ordering operator<=>(const xObjectBase & O) const { return Id <=> O.Id; }

	X_INLINE xObjectBase() {}
	X_INLINE explicit xObjectBase(const uint64_t RawId) { Id = RawId; }
	X_INLINE explicit xObjectBase(const int64_t RawId) { Id = RawId; }
	X_INLINE void operator=(uint64_t p_uint64) { Id = p_uint64; }
};
// clang-format on

class xObjectIdManager final : xNonCopyable {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();

	X_API_MEMBER uint32_t Acquire();
	X_API_MEMBER void     Release(uint32_t Id);
	X_API_MEMBER void     MarkInUse(uint32_t Id);

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
};

X_END
