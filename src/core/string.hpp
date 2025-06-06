// clang-format off
#pragma once

#include "./core_min.hpp"
#include "./optional.hpp"

#include <cstring>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

X_BEGIN

X_API           std::vector<std::string> Split(const std::string_view & s, const char * d, size_t len);
X_STATIC_INLINE std::vector<std::string> Split(const std::string_view & s, const char * d) { return Split(s, d, strlen(d)); }

X_API std::string Trim(const std::string_view & s);
X_API void		  Reverse(void * str, size_t len);

X_API void		  CStrNCpy(char * dst, size_t n, const char * src);
template <size_t N>
X_STATIC_INLINE void CStrNCpy(char (&dsrArr)[N], const char * src) {
	CStrNCpy((char *)dsrArr, N, src);
}

X_API std::string HexShowLower(const void * buffer, size_t len, size_t IndentSize = 0, bool header = false);
X_API std::string HexShow(const void * buffer, size_t len, size_t IndentSize = 0, bool header = false);
X_INLINE std::string HexShowLower(const std::string_view & v, size_t IndentSize = 0, bool header = false) { return HexShowLower(v.data(), v.size(), IndentSize, header); }
X_INLINE std::string HexShow(const std::string_view & v, size_t IndentSize = 0, bool header = false) { return HexShow(v.data(), v.size(), IndentSize, header); }

X_API void HexToStr(void * dst, const void * str, size_t len);
X_API std::string HexToStr(const void * str, size_t len);
X_INLINE std::string HexToStr(const std::string_view & view) { return HexToStr(view.data(), view.size()); }

X_API void            StrToHexLower(void * dst, const void * str, size_t len);
X_API std::string     StrToHexLower(const void * str, size_t len);
X_API std::string     StrToHexLower(const char * str);
X_INLINE std::string  StrToHexLower(const std::string_view & sv) { return StrToHexLower(sv.data(), sv.length()); }

X_API void            StrToHex(void * dst, const void * str, size_t len);
X_API std::string     StrToHex(const void * str, size_t len);
X_API std::string     StrToHex(const char * str);
X_INLINE std::string  StrToHex(const std::string_view & sv) { return StrToHex(sv.data(), sv.length()); }

X_API std::u32string  ToUtf32(const std::string_view & U8String);
X_API std::string     ToUtf8(const std::u32string_view & U32String);

X_API xOptional<std::string>      FileToStr(const char * filename);
X_INLINE xOptional<std::string>   FileToStr(const std::string & filename) { return FileToStr(filename.c_str()); }
X_API std::vector<std::string>    FileToLines(const char * filename);
X_INLINE std::vector<std::string> FileToLines(const std::string & filename) { return FileToLines(filename.c_str()); }

template <typename tIterator, typename tSeparator>
std::string JoinStr(tIterator Begin, tIterator End, tSeparator Separator) {
	std::ostringstream o;
	if (Begin != End) {
		o << *Begin++;
		for (; Begin != End; ++Begin) o << Separator << *Begin;
	}
	return o.str();
}

template <typename tIterable, typename tSeparator>
std::string JoinStr(tIterable & Iterable, tSeparator Separator) {
	return JoinStr(Iterable.begin(), Iterable.end(), Separator);
}

X_END
