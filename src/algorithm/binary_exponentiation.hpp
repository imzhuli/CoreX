#pragma once
#include "../core/core_min.hpp"

X_BEGIN

template <typename T>
X_INLINE T BinaryExponentiation(const T & Base, size_t Exponent) {
	assert(Exponent);
	auto Result = Base;
	auto Oprand = Base;
	--Exponent;
	while (Exponent) {
		if (Exponent & 0x01) {
			Result = Result * Oprand;
		}
		Oprand = Oprand * Oprand;

		Exponent >>= 1;
	}
	return Result;
}

template <typename T>
X_INLINE T BinaryExponentiation(const T & Base, size_t Exponent, const T & Unit) {
	if (!Exponent) {
		return Unit;
	}
	return BinaryExponentiation(Base, Exponent);
}

X_END
