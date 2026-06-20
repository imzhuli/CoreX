#include "./sha_rsa_sign.hpp"

#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>

X_BEGIN

struct xSha256WithRsaContext {
	mbedtls_pk_context		 KeyContext = {};
	mbedtls_entropy_context	 Entropy	= {};
	mbedtls_ctr_drbg_context CtrDrbg	= {};
	xSha256WithRsaContext() {
		mbedtls_pk_init(&KeyContext);
		mbedtls_entropy_init(&Entropy);
		mbedtls_ctr_drbg_init(&CtrDrbg);
		mbedtls_ctr_drbg_seed(&CtrDrbg, mbedtls_entropy_func, &Entropy, nullptr, 0);
	}
	~xSha256WithRsaContext() {
		mbedtls_ctr_drbg_free(&CtrDrbg);
		mbedtls_entropy_free(&Entropy);
		mbedtls_pk_free(&KeyContext);
	}
	xSha256WithRsaContext(xSha256WithRsaContext &&) = delete;  // non copyable
};

bool xSha256WithRsa::Init(const std::filesystem::path & PriKeyPath) {
	_Context = std::make_unique<xSha256WithRsaContext>();
	if (mbedtls_pk_parse_keyfile(&_Context->KeyContext, PriKeyPath.string().c_str(), nullptr, nullptr, nullptr)) {
		Reset(_Context);
		return false;
	}
	return true;
}

void xSha256WithRsa::Clean() {
	Reset(_Context);
}

bool xSha256WithRsa::Sign(void * Output, const void * Data, size_t Size) {
	ubyte Hash[32];
	mbedtls_sha256((const ubyte *)Data, Size, Hash, 0);
	size_t olen = 0;
	if (mbedtls_pk_sign(&_Context->KeyContext, MBEDTLS_MD_SHA256, Hash, sizeof(Hash), (unsigned char *)Output, 128, &olen, mbedtls_ctr_drbg_random, &_Context->CtrDrbg)) {
		return false;
	}
	return true;
}

bool xSha256WithRsa::Validate(const void * Data, size_t Size, const void * Signature) {
	ubyte Hash[32];
	mbedtls_sha256((const ubyte *)Data, Size, Hash, 0);
	if (mbedtls_pk_verify(&_Context->KeyContext, MBEDTLS_MD_SHA256, Hash, sizeof(Hash), (const ubyte *)Signature, 64)) {
		return false;
	}
	return true;
}

/* validator */

struct xSha256WithRsaValidatorContext {
	mbedtls_pk_context KeyContext = {};
	xSha256WithRsaValidatorContext() {
		mbedtls_pk_init(&KeyContext);
	}
	~xSha256WithRsaValidatorContext() {
		mbedtls_pk_free(&KeyContext);
	}
	xSha256WithRsaValidatorContext(xSha256WithRsaContext &&) = delete;	// non copyable
};

bool xSha256WithRsaValidator::Init(const std::filesystem::path & PubKeyPath) {
	_Context = std::make_unique<xSha256WithRsaValidatorContext>();
	if (mbedtls_pk_parse_public_keyfile(&_Context->KeyContext, PubKeyPath.string().c_str())) {
		Reset(_Context);
		return false;
	}
	return true;
}

void xSha256WithRsaValidator::Clean() {
	Reset(_Context);
}

bool xSha256WithRsaValidator::operator()(const void * Data, size_t Size, const void * Signature) {
	ubyte Hash[32];
	mbedtls_sha256((const ubyte *)Data, Size, Hash, 0);
	if (mbedtls_pk_verify(&_Context->KeyContext, MBEDTLS_MD_SHA256, Hash, sizeof(Hash), (const ubyte *)Signature, 64)) {
		return false;
	}
	return true;
}

X_END
