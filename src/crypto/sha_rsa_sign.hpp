#pragma once
#include "../core/core_min.hpp"
#include "../core/view.hpp"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/pk.h>

#include <filesystem>

X_BEGIN

class xSha256WithRsa {
public:
	X_API_MEMBER bool Init(const std::filesystem::path & PriKeyPath);
	X_API_MEMBER void Clean();

	X_API_MEMBER xView<ubyte> operator()(const void * Data, size_t Size);
	X_API_MEMBER bool         Validate(const void * Data, size_t Size, const void * Sha256Hash);

private:
	ubyte                    _SignResult[128];
	mbedtls_pk_context       _PriKeyContext = {};
	mbedtls_ctr_drbg_context _CtrDrbg       = {};
};

X_END
