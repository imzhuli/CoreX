#pragma once

#include "../core/core_min.hpp"

#include <compare>
X_BEGIN

class xObjectBase;
class xObjectIdManager;
class xObjectIdManagerMini;

class xObjectBase : xNonCopyable {
private:
	friend class xObjectIdManager;
	friend class xObjectIdManagerMini;

private:
	uint32_t Id       = 0;
	uint32_t RefCount = 0;

public:
	static constexpr const uint32_t MAX_OBJECT_ID = 0x0FFF'FFFFu;

public:
	X_INLINE void InitId(uint32_t NewId) {
		assert(!Id);
		Id = NewId;
	}
	X_INLINE uint32_t GetId() const { return Id; }

	X_INLINE void AddRef() {
		++RefCount;
		assert(RefCount);
	}
	X_INLINE uint32_t ReleaseRef() { return --RefCount; }

	X_INLINE bool operator==(const xObjectBase & O) const { return Id == O.Id; }
	X_INLINE std::strong_ordering operator<=>(const xObjectBase & O) const { return Id <=> O.Id; }
};

template <typename tDeleter>
class xObjectHolder {
public:
	X_INLINE xObjectHolder() = default;
	X_INLINE xObjectHolder(xObjectBase * OP, const tDeleter & D = tDeleter()) {
		if ((Target = OP)) {
			Target->AddRef();
			Deleter = D;
		}
	}
	X_INLINE xObjectHolder(const xObjectHolder & O) {
		if ((Target = O.Target)) {
			Target->AddRef();
			Deleter = O.Deleter;
		}
	}
	X_INLINE xObjectHolder(xObjectHolder && O) {
		Target  = Steal(O.Target);
		Deleter = Steal(O.Deleter);
	}
	X_INLINE ~xObjectHolder() {
		if (!Target) {
			return;
		}
		if (Target->ReleaseRef()) {
			return;
		}
		Deleter(Target);
	}

private:
	xObjectBase * Target = nullptr;
	tDeleter      Deleter;
};

class xObjectIdManager final : xNonCopyable {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();
	X_API_MEMBER void Reset();

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
	static_assert(MaxObjectId <= xObjectBase::MAX_OBJECT_ID);
};

class xObjectIdManagerMini final : xNonCopyable {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();
	X_API_MEMBER void Reset();

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
	static constexpr const size_t   AllocSize = sizeof(uint64_t) * (L0_Size + L1_Size);

public:
	static constexpr const size_t MaxObjectId = L1_Size * 64;
	static_assert(MaxObjectId <= xObjectBase::MAX_OBJECT_ID);
};

X_END
