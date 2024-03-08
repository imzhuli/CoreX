#include "./sha_rsa_sign.hpp"

#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/sha256.h>

X_BEGIN

bool xSha256WithRsa::Init(const std::filesystem::path & PriKeyPath) {
	mbedtls_pk_init(&_PriKeyContext);
	auto Cleaner = xScopeGuard([&] { mbedtls_pk_free(&_PriKeyContext); });

	mbedtls_ctr_drbg_init(&_CtrDrbg);
	auto CtrCleaner = xScopeGuard([&] { mbedtls_ctr_drbg_free(&_CtrDrbg); });

	if (auto ret = mbedtls_pk_parse_keyfile(&_PriKeyContext, PriKeyPath.string().c_str(), nullptr, nullptr, nullptr)) {
		X_DEBUG_PRINTF("failed\n  ! mbedtls_pk_parse_keyfile returned -0x%04x\n", -ret);
		return false;
	}

	Cleaner.Dismiss();
	CtrCleaner.Dismiss();
	return true;
}

void xSha256WithRsa::Clean() {
	mbedtls_ctr_drbg_free(&_CtrDrbg);
	mbedtls_pk_free(&_PriKeyContext);
	return;
}

xArrayView<ubyte> xSha256WithRsa::operator()(const void * Data, size_t Size) {
	ubyte Hash[32];
	mbedtls_sha256((const ubyte *)Data, Size, Hash, 0);

	size_t olen = 0;
	if (auto ret = mbedtls_pk_sign(&_PriKeyContext, MBEDTLS_MD_SHA256, Hash, sizeof(Hash), _SignResult, sizeof(_SignResult), &olen, mbedtls_ctr_drbg_random, &_CtrDrbg)) {
		X_DEBUG_PRINTF("failed\n  ! mbedtls_pk_encrypt returned -0x%04x\n", -ret);
		return {};
	}
	return { _SignResult, olen };
}

bool xSha256WithRsa::Validate(const void * Data, size_t Size, const void * Signature) {
	ubyte Hash[32];
	mbedtls_sha256((const ubyte *)Data, Size, Hash, 0);
	if (auto ret = mbedtls_pk_verify(&_PriKeyContext, MBEDTLS_MD_SHA256, Hash, sizeof(Hash), (const ubyte *)Signature, 64)) {
		X_DEBUG_PRINTF("failed\n  ! mbedtls_pk_verify returned -0x%04x\n", -ret);
		return false;
	}
	return true;
}

/* validator */

bool xSha256WithRsaValidator::Init(const std::filesystem::path & PubKeyPath) {
	mbedtls_pk_init(&_PubKeyContext);
	auto Cleaner = xScopeGuard([&] { mbedtls_pk_free(&_PubKeyContext); });

	mbedtls_ctr_drbg_init(&_CtrDrbg);
	auto CtrCleaner = xScopeGuard([&] { mbedtls_ctr_drbg_free(&_CtrDrbg); });

	if (auto ret = mbedtls_pk_parse_public_keyfile(&_PubKeyContext, PubKeyPath.string().c_str())) {
		X_DEBUG_PRINTF("failed\n  ! mbedtls_pk_parse_public_keyfile returned -0x%04x\n", -ret);
		return false;
	}

	Cleaner.Dismiss();
	CtrCleaner.Dismiss();
	return true;
}

void xSha256WithRsaValidator::Clean() {
	mbedtls_ctr_drbg_free(&_CtrDrbg);
	mbedtls_pk_free(&_PubKeyContext);
	return;
}

bool xSha256WithRsaValidator::operator()(const void * Data, size_t Size, const void * Signature) {
	ubyte Hash[32];
	mbedtls_sha256((const ubyte *)Data, Size, Hash, 0);
	if (auto ret = mbedtls_pk_verify(&_PubKeyContext, MBEDTLS_MD_SHA256, Hash, sizeof(Hash), (const ubyte *)Signature, 64)) {
		X_DEBUG_PRINTF("failed\n  ! mbedtls_pk_verify returned -0x%04x\n", -ret);
		return false;
	}
	return true;
}

X_END
