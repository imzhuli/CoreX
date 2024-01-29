#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

#include <core/executable.hpp>
#include <core/string.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace xel;

std::vector<ubyte> RsaEncrypt(const std::vector<ubyte> & input, const std::string & pubKeyFile) {
	auto pub = mbedtls_pk_context();
	mbedtls_pk_init(&pub);
	auto pub_cleaner = xScopeGuard([&] { mbedtls_pk_free(&pub); });

	if (auto ret = mbedtls_pk_parse_public_keyfile(&pub, pubKeyFile.c_str())) {
		X_DEBUG_PRINTF("mbedtls_pk_parse_public_keyfile returned error: %0x", -ret);
		return {};
	}

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	auto ctr_clearn = xScopeGuard([&] { mbedtls_ctr_drbg_free(&ctr_drbg); });

	std::vector<ubyte> buf(MBEDTLS_MPI_MAX_SIZE);
	size_t             olen = 0;
	if (auto ret =
			mbedtls_pk_encrypt(&pub, input.data(), input.size(), buf.data(), &olen, buf.size(), mbedtls_ctr_drbg_random, &ctr_drbg)) {
		printf(" failed\n  ! mbedtls_pk_encrypt returned -0x%04x\n", -ret);
		return {};
	}
	buf.resize(olen);
	return buf;
}

std::vector<ubyte> RsaDecrypt(const std::vector<ubyte> & input, const std::string & priKeyFile) {
	auto pri = mbedtls_pk_context();
	mbedtls_pk_init(&pri);
	auto pri_cleaner = xScopeGuard([&] { mbedtls_pk_free(&pri); });

	if (auto ret = mbedtls_pk_parse_keyfile(&pri, priKeyFile.c_str(), nullptr, nullptr, nullptr)) {
		X_DEBUG_PRINTF("mbedtls_pk_parse_keyfile returned error: %0x", -ret);
		return {};
	}

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	auto ctr_clearn = xScopeGuard([&] { mbedtls_ctr_drbg_free(&ctr_drbg); });

	std::vector<ubyte> buf(MBEDTLS_MPI_MAX_SIZE);
	size_t             olen = 0;
	if (auto ret =
			mbedtls_pk_decrypt(&pri, input.data(), input.size(), buf.data(), &olen, buf.size(), mbedtls_ctr_drbg_random, &ctr_drbg)) {
		printf(" failed\n  ! mbedtls_pk_decrypt returned -0x%04x\n", -ret);
		return {};
	}
	buf.resize(olen);
	return buf;
}

std::vector<ubyte> RsaSignSha256(const std::vector<ubyte> & Data, const std::string & priKeyFile) {

	ubyte hash[32];
	mbedtls_sha256(Data.data(), Data.size(), hash, 0);

	auto pri = mbedtls_pk_context();
	mbedtls_pk_init(&pri);
	auto pri_cleaner = xScopeGuard([&] { mbedtls_pk_free(&pri); });

	if (auto ret = mbedtls_pk_parse_keyfile(&pri, priKeyFile.c_str(), nullptr, nullptr, nullptr)) {
		X_DEBUG_PRINTF("mbedtls_pk_parse_keyfile returned error: %0x", -ret);
		return {};
	}

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ctr_drbg_init(&ctr_drbg);
	auto ctr_clearn = xScopeGuard([&] { mbedtls_ctr_drbg_free(&ctr_drbg); });

	auto   buf  = std::vector<ubyte>(MBEDTLS_PK_SIGNATURE_MAX_SIZE);
	size_t olen = 0;
	if (auto ret = mbedtls_pk_sign(&pri, MBEDTLS_MD_SHA256, hash, 0, buf.data(), buf.size(), &olen, mbedtls_ctr_drbg_random, &ctr_drbg)) {
		X_DEBUG_PRINTF("mbedtls_pk_sign returned error: %0x", -ret);
		return {};
	}
	buf.resize(olen);
	return buf;
}

bool RsaValidate(const std::vector<ubyte> & input, const std::string & pubKeyFile) {
	auto pub = mbedtls_pk_context();
	mbedtls_pk_init(&pub);
	auto pub_cleaner = xScopeGuard([&] { mbedtls_pk_free(&pub); });

	if (auto ret = mbedtls_pk_parse_public_keyfile(&pub, pubKeyFile.c_str())) {
		X_DEBUG_PRINTF("mbedtls_pk_parse_public_keyfile returned error: %0x", -ret);
		return false;
	}

	return false;
}

int main(int argc, char * argv[]) {

	auto CL = xCommandLine(
		argc, argv,
		{
			{ 'e', "encrypt-input", "encrypt-input", true },
			{ 'b', "base64-input", "base64-input", true },
			{ 'k', "key", "key", true },
		}
	);

	auto OptPK = CL["key"];
	if (!OptPK()) {
		cerr << "missing key file name" << endl;
		return -1;
	}

	auto OptE = CL["encrypt-input"];
	if (OptE()) {
		auto Source = std::vector<ubyte>(OptE->size());
		memcpy(Source.data(), OptE->data(), OptE->size());
		auto E = RsaEncrypt(Source, *OptPK + ".pub");
		cout << "After encryption: " << StrToHex(E.data(), E.size()) << endl;

		auto B = std::string();
		B.resize(E.size() * 2);

		auto BL = size_t(0);
		mbedtls_base64_encode((ubyte *)B.data(), B.size(), &BL, E.data(), E.size());
		B.resize(BL);

		cout << "Encrypted: " << B << endl;

		auto sign = RsaSignSha256(Source, *OptPK + ".key");
		cout << HexShow(sign.data(), sign.size()) << endl;

		B.resize(sign.size() * 2);
		BL = size_t(0);
		mbedtls_base64_encode((ubyte *)B.data(), B.size(), &BL, sign.data(), sign.size());
		B.resize(BL);
		cout << HexShow(B.data(), B.size()) << endl;
	}

	auto OptB64 = CL["base64-input"];
	if (OptB64()) {
		auto D  = std::vector<ubyte>(OptB64->size());
		auto DL = size_t(0);
		if (auto ret = mbedtls_base64_decode(D.data(), D.size(), &DL, (ubyte *)OptB64->data(), OptB64->size())) {
			printf(" failed\n  ! mbedtls_base64_decode returned -0x%04x\n", -ret);
		}
		D.resize(DL);

		cout << "Before decryption: " << StrToHex(D.data(), D.size()) << endl;
		auto Decrypted = RsaDecrypt(D, *OptPK + ".key");
		cout << HexShow(Decrypted.data(), Decrypted.size()) << endl;
	}

	return 0;
}
