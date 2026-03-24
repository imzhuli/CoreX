#include "./key.hpp"

#include <mbedtls/pk.h>

X_BEGIN

static int FillSeven(void *, unsigned char * T, size_t S) {
	memset(T, 7, S);
	return 0;
}

bool xCryptoKey::Init(const char * PemFile) {
	RuntimeAssert(!Key);
	return false;
}

bool xCryptoKey::Init(const char * PemFile, const char * Pass) {
	RuntimeAssert(!Key);
	mbedtls_pk_context * pk = new mbedtls_pk_context;
	mbedtls_pk_init(pk);

	int ret;
	ret = mbedtls_pk_parse_keyfile(pk, PemFile, Pass, &FillSeven, nullptr);
	if (ret != 0) {
		delete pk;
		return false;
	}
	return true;
}

void xCryptoKey::Clean() {
	assert(Key);
	auto pk = (mbedtls_pk_context *)Steal(Key);
	mbedtls_pk_free(pk);
	delete pk;
}

X_END
