#include "./logger.hpp"

#include "./core_stream.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <string>
#include <thread>

X_BEGIN

namespace fs = std::filesystem;

static constexpr const char gcHint[] = {
	'V', 'D', 'I', 'W', 'E', 'F', 'Q',
};

static bool EnsureDirectoryExists(const fs::path & path) {
	try {
		fs::path dir = path.parent_path();

		if (dir.empty()) {
			return true;
		}
		if (!fs::exists(dir)) {
			fs::create_directories(dir);
		}
	} catch (const fs::filesystem_error & e) {
		return false;
	}
	return true;
}

xLogger::xLogger() {
}

xLogger::~xLogger() {
}

xBaseLogger::xBaseLogger() {
}

xBaseLogger::~xBaseLogger() {
	assert(!_LogFile);
}

bool xBaseLogger::Init(const char * PathPtr) {
	if (!PathPtr) {
		return false;
	};
	if (!EnsureDirectoryExists(_LogFilename = PathPtr)) {
		return false;
	}
	return _LogFile = fopen(_LogFilename.string().c_str(), "at");
}

void xBaseLogger::Clean() {
	fclose(Steal(_LogFile));
	Reset(_LogFilename);
	SetLogLevel(eLogLevel::Debug);
}

void xBaseLogger::Log(eLogLevel ll, const char * fmt, ...) {
	if (ll < _LogLevel) {
		return;
	}

	std::tm     brokenTime;
	std::time_t now = std::time(nullptr);
	XelLocalTime(&now, &brokenTime);

	std::hash<std::thread::id> hasher;

	va_list vaList;
	va_start(vaList, fmt);

	do {  // synchronized block
		auto guard = std::lock_guard{ _SyncMutex };
		fprintf(
			_LogFile, "%c<%016zx>%02d%02d%02d:%02d%02d%02d ", gcHint[static_cast<size_t>(ll)], hasher(std::this_thread::get_id()), brokenTime.tm_year + 1900 - 2000,
			brokenTime.tm_mon + 1, brokenTime.tm_mday, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
		);
		vfprintf(_LogFile, fmt, vaList);
		fputc('\n', _LogFile);
	} while (false);
	fflush(_LogFile);

	va_end(vaList);
	return;
}

void xStdLogger::Log(eLogLevel ll, const char * fmt, ...) {
	if (ll < _LogLevel) {
		return;
	}

	auto _LogFile = stdout;
	if (ll == eLogLevel::Error || ll == eLogLevel::Fatal) {
		_LogFile = stderr;
	}

	std::tm     brokenTime;
	std::time_t now = std::time(nullptr);
	XelLocalTime(&now, &brokenTime);

	std::hash<std::thread::id> hasher;

	va_list vaList;
	va_start(vaList, fmt);

	do {  // synchronized block
		auto guard = std::lock_guard{ _SyncMutex };
		fprintf(
			_LogFile, "%c<%016zx>%02d%02d%02d:%02d%02d%02d ", gcHint[static_cast<size_t>(ll)], hasher(std::this_thread::get_id()), brokenTime.tm_year + 1900 - 2000,
			brokenTime.tm_mon + 1, brokenTime.tm_mday, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
		);
		vfprintf(_LogFile, fmt, vaList);
		fputc('\n', _LogFile);
	} while (false);
	fflush(_LogFile);

	va_end(vaList);
	return;
}

xMemoryLogger::xMemoryLogger() {
}

xMemoryLogger::~xMemoryLogger() {
	assert(!_LogBufferPtr);
}

bool xMemoryLogger::Init(size32_t MaxLineNumber, size32_t MaxLineSize) {
	assert(MaxLineNumber > 1);
	assert(MaxLineSize > 1);
	assert(!_LogBufferPtr);

	++MaxLineNumber;
	_LineSize              = MaxLineSize;
	_RealLineSize          = LineLeadBufferSize + _LineSize + ExtraSize;
	size_t TotalBufferSize = MaxLineNumber * _RealLineSize;
	_LogBufferPtr          = (char *)malloc(TotalBufferSize);
	if (!_LogBufferPtr) {
		Reset(_LineSize);
		Reset(_RealLineSize);
		return false;
	}
	_CurrentLineIndex = 0;
	_LineNumber       = MaxLineNumber;
	memset(_LogBufferPtr, 0, TotalBufferSize);
	return true;
}

void xMemoryLogger::Clean() {
	assert(_LogBufferPtr);
	free(Steal(_LogBufferPtr));
	Reset(_LineSize);
	Reset(_RealLineSize);
	Reset(_LineNumber);
	Reset(_CurrentLineIndex);
	SetLogLevel(eLogLevel::Debug);
}

void xMemoryLogger::Log(eLogLevel ll, const char * fmt, ...) {
	std::tm     brokenTime;
	std::time_t now = std::time(nullptr);
#ifdef _MSC_VER
	localtime_s(&brokenTime, &now);
#else
	localtime_r(&now, &brokenTime);
#endif

	va_list vaList;
	va_start(vaList, fmt);

	char LineLead[LineLeadBufferSize];
	int  LineLeadSize = snprintf(
        LineLead, Length(LineLead), "%c<%016zx>%02d%02d%02d:%02d%02d%02d ", gcHint[static_cast<size_t>(ll)], std::hash<std::thread::id>{}(std::this_thread::get_id()),
        brokenTime.tm_year + 1900 - 2000, brokenTime.tm_mon + 1, brokenTime.tm_mday, brokenTime.tm_hour, brokenTime.tm_min, brokenTime.tm_sec
    );

	do {  // synchronized block
		auto Guard = xSpinlockGuard{ _Spinlock };

		assert(_LogBufferPtr);
		char *        LineStart = _LogBufferPtr + _CurrentLineIndex * _RealLineSize;
		xStreamWriter S(LineStart);
		S.W(LineLead, LineLeadSize);
		int LogLength = vsnprintf((char *)S(), _LineSize, fmt, vaList);
		if (LogLength > 0) {
			S.Skip(std::min((size_t)LogLength, _LineSize - 1));
		}
		S.W('\n');
		S.W('\0');
		// printf("---offset:%d/%d/%d---", (int)S.Offset(), (int)LogLength, (int)_LineSize);

		++_CurrentLineIndex;
		_CurrentLineIndex %= _LineNumber;
	} while (false);

	va_end(vaList);
	return;
}

void xMemoryLogger::Output(FILE * fp) {
	if (!fp) {
		return;
	}
	auto Guard = xSpinlockGuard{ _Spinlock };

	size_t StartIndex = _CurrentLineIndex + 1;
	while (StartIndex != _CurrentLineIndex) {
		auto LineStart = _LogBufferPtr + StartIndex * _RealLineSize;
		if (*LineStart) {
			fprintf(fp, "%s", LineStart);
		}
		++StartIndex;
		StartIndex %= _LineNumber;
	}
}

X_END
