#pragma once
#include "../core/core_min.hpp"
#include "../core/view.hpp"

#include <string>

X_BEGIN

X_API std::string Base64Encode(const void * ToEncode, size_t Size);
X_API std::string Base64Decode(const void * ToDecode, size_t Size);

X_END
