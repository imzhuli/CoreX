
#include "./core_min.hpp"

#include <cstdarg>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <string>

#ifdef X_SYSTEM_ANDROID
#include <android/log.h>
#endif

// clang-format off
X_COMMON_BEGIN

static_assert(sizeof(xVariable) == sizeof(xVariable::B));
static_assert(std::is_trivially_copyable_v<xVariable>);

void Breakpoint() {}

static auto DebugPrintMutex = std::mutex();
void DebugPrintf(const char * Path, size_t Line, const char * FunctionName, const char * Fmt, ...) {
	auto SS = std::ostringstream();

	auto Filename = std::filesystem::path(Path).filename().string();
	for (auto & C : Filename) {
		if (C == '\\') {
			SS << "\\\\";
			continue;
		}
		if (C == '%') {
			SS << "%%";
			continue;
		}
		SS << C;
	}
	SS << ":" << Line << " @" << FunctionName << " " << Fmt << "\n";
	auto FormatString = SS.str();

	va_list args;
	va_start(args, Fmt);
	do {
		auto Guard = std::lock_guard(DebugPrintMutex);
#ifdef X_SYSTEM_ANDROID
		__android_log_vprint(ANDROID_LOG_DEBUG, FormatString.c_str(), args);
#else
		vprintf(FormatString.c_str(), args);
#endif
	} while (false);
	va_end(args);
}

static auto ErrorPrintfMutex = std::mutex();
void ErrorPrintf(const char * Path, size_t Line, const char * FunctionName, const char * Fmt, ...) {
	auto SS = std::ostringstream();

	auto Filename = std::filesystem::path(Path).filename().string();
	for (auto & C : Filename) {
		if (C == '\\') {
			SS << "\\\\";
			continue;
		}
		if (C == '%') {
			SS << "%%";
			continue;
		}
		SS << C;
	}
	SS << ":" << Line << " @" << FunctionName << " " << Fmt << "\n";
	auto FormatString = SS.str();

	va_list args;
	va_start(args, Fmt);
	do {
		auto Guard = std::lock_guard(ErrorPrintfMutex);
#ifdef X_SYSTEM_ANDROID
		__android_log_vprint(ANDROID_LOG_ERROR, FormatString.c_str(), args);
#else
		vfprintf(stderr, FormatString.c_str(), args);
#endif
	} while (false);
	va_end(args);
}

X_COMMON_END
