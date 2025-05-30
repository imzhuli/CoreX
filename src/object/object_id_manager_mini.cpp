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

X_STATIC_INLINE uint32_t FirstValidSlot(uint64_t B2) {
	return (uint32_t)std::countr_one(B2);
}

bool xObjectIdManagerMini::Init() {
	Bitmap = (uint64_t *)malloc(AllocSize);
	if (!Bitmap) {
		return false;
	}
	memset(Bitmap, 0, AllocSize);
	return true;
}

void xObjectIdManagerMini::Clean() {
	free(Bitmap);
	Bitmap = nullptr;
}

void xObjectIdManagerMini::Reset() {
	memset(Bitmap, 0, AllocSize);
}

uint32_t xObjectIdManagerMini::Acquire() {
	uint_fast32_t Index0 = L0_Start;
	uint_fast32_t B1     = FirstValidSlot(Bitmap[Index0]);
	if (B1 == 64) {
		return 0;
	}
	uint_fast32_t Index1 = L1_Start + B1;
	uint_fast32_t B2     = FirstValidSlot(Bitmap[Index1]);

	if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index1] |= (BASE_ONE << B2))) {
		Bitmap[Index0] |= (BASE_ONE << B1);
	}
	auto NewId = (uint32_t)((B1 << 6) + B2 + 1);
	return NewId;
}

bool xObjectIdManagerMini::Acquire(uint32_t Id) {
	assert(Id && Id <= MaxObjectId);

	Id              -= 1;
	uint_fast32_t B2 = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B1 = Id & 0x3FU;

	uint_fast32_t Index0 = L0_Start;
	uint_fast32_t Index1 = L1_Start + B1;

	uint64_t FinestBitValue = BASE_ONE << B2;
	if (Bitmap[Index1] & FinestBitValue) {  // already in use
		return false;
	}

	if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index1] |= (FinestBitValue))) {
		Bitmap[Index0] |= (BASE_ONE << B1);
	}
	return true;
}

void xObjectIdManagerMini::Release(uint32_t Id) {
	assert(Id && Id <= MaxObjectId);

	Id              -= 1;
	uint_fast32_t B2 = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B1 = Id & 0x3FU;

	uint_fast32_t Index0 = L0_Start;
	uint_fast32_t Index1 = L1_Start + B1;

	assert(Bitmap[Index1] & (BASE_ONE << B2));
	Bitmap[Index0] &= ~(BASE_ONE << B1);
	Bitmap[Index1] &= ~(BASE_ONE << B2);
}

bool xObjectIdManagerMini::IsInUse(uint32_t Id) const {
	assert(Id && Id <= MaxObjectId);

	--Id;
	auto FinestBitSetOffset = L1_Start + (Id >> 6);
	auto FinestBitValue     = BASE_ONE << (Id & 0x3FU);
	return Bitmap[FinestBitSetOffset] & FinestBitValue;
}

X_END
