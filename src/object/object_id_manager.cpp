#include "./object.hpp"

#include <bit>
#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <limits>

// adsfasdf
#include <iostream>
using namespace std;

X_BEGIN

X_STATIC_INLINE uint32_t FirstValidSlot(uint64_t B3) {
	return (uint32_t)std::countr_one(B3);
}

bool xObjectIdManager::Init() {
	Bitmap = (uint64_t *)malloc(AllocSize);
	if (!Bitmap) {
		return false;
	}
	memset(Bitmap, 0, AllocSize);
	return true;
}

void xObjectIdManager::Clean() {
	free(Bitmap);
	Bitmap = nullptr;
}

void xObjectIdManager::Reset() {
	memset(Bitmap, 0, AllocSize);
}

uint32_t xObjectIdManager::Acquire() {
	uint_fast32_t B0     = 0;
	uint_fast32_t Index0 = L0_Start + B0;
	uint_fast32_t B1     = FirstValidSlot(Bitmap[Index0]);
	if (B1 == 64) {
		return 0;
	}
	uint_fast32_t Index1 = L1_Start + B1;

	uint_fast32_t B2     = FirstValidSlot(Bitmap[Index1]);
	uint_fast32_t Index2 = L2_Start + (B1 << 6) + B2;
	uint_fast32_t B3     = FirstValidSlot(Bitmap[Index2]);

	if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index2] |= (BASE_ONE << B3))) {
		if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index1] |= (BASE_ONE << B2))) {
			Bitmap[Index0] |= (BASE_ONE << B1);
		}
	}
	auto NewId = (uint32_t)((B1 << 12) + (B2 << 6) + B3 + 1);
	return NewId;
}

void xObjectIdManager::Release(uint32_t Id) {
	assert(Id && Id <= MaxObjectId);

	Id              -= 1;
	uint_fast32_t B3 = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B2 = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B1 = Id & 0x3FU;

	uint_fast32_t B0     = 0;
	uint_fast32_t Index0 = L0_Start + B0;
	uint_fast32_t Index1 = L1_Start + B1;
	uint_fast32_t Index2 = L2_Start + B1 * 64 + B2;

	assert(Bitmap[Index2] & (BASE_ONE << B3));
	Bitmap[Index0] &= ~(BASE_ONE << B1);
	Bitmap[Index1] &= ~(BASE_ONE << B2);
	Bitmap[Index2] &= ~(BASE_ONE << B3);
}

bool xObjectIdManager::MarkInUse(uint32_t Id) {
	assert(Id && Id <= MaxObjectId);

	Id              -= 1;
	uint_fast32_t B3 = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B2 = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B1 = Id & 0x3FU;

	uint_fast32_t B0     = 0;
	uint_fast32_t Index0 = L0_Start + B0;
	uint_fast32_t Index1 = L1_Start + B1;
	uint_fast32_t Index2 = L2_Start + B1 * 64 + B2;

	uint64_t FinestBitValue = BASE_ONE << B3;
	if (Bitmap[Index2] & FinestBitValue) {  // already in use
		return false;
	}

	if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index2] |= FinestBitValue)) {
		if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index1] |= (BASE_ONE << B2))) {
			Bitmap[Index0] |= (BASE_ONE << B1);
		}
	}
	return true;
}

X_END
