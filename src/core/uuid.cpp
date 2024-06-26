#include "./uuid.hpp"

#include <cstdint>
#include <cstdio>

#if defined(_WIN32)
#include <windows.h>
#include <wincrypt.h>
#endif

X_BEGIN

/**
 * Copyright (c) 2018 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

enum { UUID4_ESUCCESS = 0, UUID4_EFAILURE = -1 };

static constexpr size_t UUID4_LEN = 33;

struct xUuidSeed {
	uint64_t seed[2];
};

// static uint64_t xorshift128plus(uint64_t *s)
// {
// 	/* http://xorshift.di.unimi.it/xorshift128plus.c */
// 	uint64_t s1 = s[0];
// 	const uint64_t s0 = s[1];
// 	s[0] = s0;
// 	s1 ^= s1 << 23;
// 	s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);
// 	return s[1] + s0;
// }

static int uuid4_init(xUuidSeed & Ret) {
	auto & seed = Ret.seed;
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
	int    res;
	FILE * fp = fopen("/dev/urandom", "rb");
	if (!fp) {
		return UUID4_EFAILURE;
	}
	res = fread(seed, 1, sizeof(seed), fp);
	fclose(fp);
	if (res != sizeof(seed)) {
		return UUID4_EFAILURE;
	}

#elif defined(_WIN32)
	int        res;
	HCRYPTPROV hCryptProv;
	res = CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	if (!res) {
		return UUID4_EFAILURE;
	}
	res = CryptGenRandom(hCryptProv, (DWORD)sizeof(seed), (PBYTE)seed);
	CryptReleaseContext(hCryptProv, 0);
	if (!res) {
		return UUID4_EFAILURE;
	}
#else
#error "unsupported platform"
#endif
	return UUID4_ESUCCESS;
}

static bool uuid4_generate(char * dst) {
	xUuidSeed Seed;
	if (UUID4_ESUCCESS != uuid4_init(Seed)) {
		return false;
	}
	auto & seed = Seed.seed;

	static const char * Template = "xxxxxxxxxxxx4xxxyxxxxxxxxxxxxxxx";
	static const char * chars    = "0123456789abcdef";
	union {
		unsigned char b[16];
		uint64_t      word[2];
	} s;
	const char * p;
	int          i, n;

	/* get random: disabled since each generation calls system random engine */
	s.word[0] = seed[0];
	s.word[1] = seed[1];

	/* build string */
	p = Template;
	i = 0;
	while (*p) {
		n = s.b[i >> 1];
		n = (i & 1) ? (n >> 4) : (n & 0xf);
		switch (*p) {
			case 'x':
				*dst = chars[n];
				i++;
				break;
			case 'y':
				*dst = chars[(n & 0x3) + 8];
				i++;
				break;
			default:
				*dst = *p;
		}
		dst++, p++;
	}
	*dst = '\0';
	return true;
}

static_assert(sizeof(xUuid) == sizeof(xUuid::xRawType));

bool xUuid::Generate() {
	char Data[UUID4_LEN];
	if (!uuid4_generate(Data)) {
		memset(_Data, 0, sizeof(_Data));
		return false;
	}
	HexToStr(_Data, Data, 32);
	return true;
}

X_END