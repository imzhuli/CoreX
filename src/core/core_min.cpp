#include "./core_min.hpp"

#include <filesystem>
#include <mutex>
#include <sstream>
#include <string>

#ifdef X_SYSTEM_ANDROID
#include <android/log.h>
#endif

X_COMMON_BEGIN

static auto DebugPrintMutex = std::mutex();

void Breakpoint() {
}

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

X_COMMON_END