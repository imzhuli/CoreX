#pragma once
#include "../core/core_min.hpp"
#include "./char_range.hpp"

#include <algorithm>

X_BEGIN

X_STATIC_INLINE bool IsUnicodeIdStart(char32_t c) {
	auto Begin = UnicodeIdStart;
	auto End = UnicodeIdStart + Length(UnicodeIdStart);

	auto Key = xCharRange{ c, 0 };
	auto Iter = std::upper_bound(Begin, End, Key);
	if (Iter == Begin) {
		return false;
	}
	--Iter;
	return c >= Iter->Start && c <= Iter->End;
}

X_STATIC_INLINE bool IsUnicodeIdContinue(char32_t c) {
	auto Begin = UnicodeIdContinue;
	auto End = UnicodeIdContinue + Length(UnicodeIdContinue);

	auto Key = xCharRange{ c, 0 };
	auto Iter = std::upper_bound(Begin, End, Key);
	if (Iter == Begin) {
		return false;
	}
	--Iter;
	return c >= Iter->Start && c <= Iter->End;
}

X_STATIC_INLINE bool IsAsciiUpperCase(char32_t c) { return (c >= 'A' && c <= 'Z'); }

X_STATIC_INLINE bool IsAsciiLowerCase(char32_t c) { return (c >= 'a' && c <= 'z'); }

X_STATIC_INLINE bool IsDigit(char32_t c) { return (c >= '0' && c <= '9'); }

X_STATIC_INLINE bool IsHexDigit(char32_t c) { return IsDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

X_STATIC_INLINE bool IsBinaryDigit(char32_t c) { return c == '0' || c == '1'; }

X_STATIC_INLINE bool IsAsciiChar(char32_t c) { return IsAsciiUpperCase(c) || IsAsciiLowerCase(c); }

X_STATIC_INLINE bool IsAsciiAlplhanumericChar(char32_t c) { return IsAsciiChar(c) || IsDigit(c); }

X_STATIC_INLINE bool IsAsciiIdentifierChar(char32_t c) { return IsAsciiAlplhanumericChar(c) || c == '_'; }

X_STATIC_INLINE bool IsSymbol(char32_t c) { return c != '_' && ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~') || c == '\t' || c == ' '); }

X_STATIC_INLINE bool IsControl(char32_t p_char) { return (p_char <= 0x001f) || (p_char >= 0x007f && p_char <= 0x009f); }

X_STATIC_INLINE bool IsWhiteSpace(char32_t p_char) {
	return (p_char == ' ') || (p_char == 0x00a0) || (p_char == 0x1680) || (p_char >= 0x2000 && p_char <= 0x200a) || (p_char == 0x202f) || (p_char == 0x205f) || (p_char == 0x3000) ||
		(p_char == 0x2028) || (p_char == 0x2029) || (p_char >= 0x0009 && p_char <= 0x000d) || (p_char == 0x0085);
}

X_STATIC_INLINE bool IsLineBreak(char32_t p_char) { return (p_char >= 0x000a && p_char <= 0x000d) || (p_char == 0x0085) || (p_char == 0x2028) || (p_char == 0x2029); }

X_STATIC_INLINE bool IsPunct(char32_t p_char) {
	return (p_char >= ' ' && p_char <= '/') || (p_char >= ':' && p_char <= '@') || (p_char >= '[' && p_char <= '^') || (p_char == '`') || (p_char >= '{' && p_char <= '~') ||
		(p_char >= 0x2000 && p_char <= 0x206f) || (p_char >= 0x3000 && p_char <= 0x303f);
}

X_STATIC_INLINE bool IsUnderscore(char32_t p_char) { return (p_char == '_'); }

X_END
