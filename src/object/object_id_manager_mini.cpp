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

X_STATIC_INLINE uint32_t FirstValidSlot(uint64_t B) {
	return (uint32_t)std::countr_one(B);
}

bool xObjectIdManagerMini::Init() {
	Bitmap = (uint64_t *)malloc(sizeof(uint64_t) * AllocSize);
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

uint32_t xObjectIdManagerMini::Acquire() {
	uint_fast32_t B0     = 0;
	uint_fast32_t Index0 = L0_Start + B0;
	uint_fast32_t B1     = FirstValidSlot(Bitmap[Index0]);
	if (B1 == 64) {
		return 0;
	}
	uint_fast32_t Index1 = L1_Start + B1;
	uint_fast32_t B      = FirstValidSlot(Bitmap[Index1]);

	if (std::numeric_limits<uint64_t>::max() == (Bitmap[Index1] |= (BASE_ONE << B))) {
		Bitmap[Index0] |= (BASE_ONE << B1);
	}
	auto NewId = (uint32_t)((B1 << 6) + B + 1);
	return NewId;
}

void xObjectIdManagerMini::Release(uint32_t Id) {
	assert(Id && Id <= MaxObjectId);

	Id              -= 1;
	uint_fast32_t B  = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B1 = Id & 0x3FU;

	uint_fast32_t B0     = 0;
	uint_fast32_t Index0 = L0_Start + B0;
	uint_fast32_t Index1 = L1_Start + B1;

	assert(Bitmap[Index1] & (BASE_ONE << B));
	Bitmap[Index0] &= ~(BASE_ONE << B1);
	Bitmap[Index1] &= ~(BASE_ONE << B);
}

void xObjectIdManagerMini::MarkInUse(uint32_t Id) {
	assert(Id && Id <= MaxObjectId);

	Id              -= 1;
	uint_fast32_t B  = Id & 0x3FU;
	Id             >>= 6;
	uint_fast32_t B1 = Id & 0x3FU;

	uint_fast32_t B0     = 0;
	uint_fast32_t Index0 = L0_Start + B0;
	uint_fast32_t Index1 = L1_Start + B1;

	assert(Bitmap[Index1] & (BASE_ONE << B));
	Bitmap[Index0] |= (BASE_ONE << B1);
	Bitmap[Index1] |= (BASE_ONE << B);
}

X_END
