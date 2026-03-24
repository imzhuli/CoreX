#pragma once
#include "../core/core_min.hpp"

#include <vector>

X_BEGIN

class xCryptoKey final : xNonCopyable {

public:
	X_INLINE xCryptoKey() = default;
	X_INLINE ~xCryptoKey() { assert(!Key); }

	X_API_MEMBER bool Init(const char * PemFile);                               // load public key
	X_API_MEMBER bool Init(const char * PemFile, const char * Pass = nullptr);  // load private key
	X_API_MEMBER void Clean();

	X_INLINE void * GetKey() const { return Key; }

private:
	void * Key = nullptr;
};

X_END
