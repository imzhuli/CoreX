// clang-format off
#include "./core_min.hpp"

#include <cstdarg>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <string>

#ifdef X_SYSTEM_ANDROID
#include <android/log.h>
#endif

X_COMMON_BEGIN

void Breakpoint() {}

void QuickExit(int ExitCode) {
#ifdef X_SYSTEM_DARWIN
	_Exit(ExitCode);
#else
	quick_exit(ExitCode);
#endif
}

void QuickExit(const char * PErrorMessage, int ExitCode){
	if (PErrorMessage) {
		fprintf(stderr, "%s\n", PErrorMessage);
	}
	QuickExit(ExitCode);
}

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
	fflush(stdout);
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
	fflush(stderr);
}

static auto FatalPrintfMutex = std::mutex();
void FatalPrintf(const char * Path, size_t Line, const char * FunctionName, const char * Fmt, ...) {
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
		auto Guard = std::lock_guard(FatalPrintfMutex);
#ifdef X_SYSTEM_ANDROID
		__android_log_vprint(ANDROID_LOG_FATAL, FormatString.c_str(), args);
#else
		vfprintf(stderr, FormatString.c_str(), args);
#endif
	} while (false);
	va_end(args);
	fflush(stderr);
	QuickExit(EXIT_FAILURE);
}

namespace __common_detail__ {
	xCompilerUnitEntry::xCompilerUnitEntry() {}
}

X_COMMON_END

// External checkers:
#ifdef __X_CORE_CHECKER__
#error "multiple checker area"
#else
#define __X_CORE_CHECKER__

#include "./core_stream.checker.inl"

#undef __X_CORE_CHECKER__
#endif
