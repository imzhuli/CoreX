#pragma once
#include "./core_min.hpp"
#include "./thread.hpp"

#include <atomic>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <mutex>

X_BEGIN
enum struct eLogLevel : int_fast32_t {  // !!! Note: implementation may use the value as index of output string,
										// so, ALWAYS increase values by ONE,
	Verbose = 0,
	Debug   = 1,
	Info    = 2,
	Warning = 3,
	Error   = 4,
	Quiet   = 1024,
};

class xLogger : xAbstract {
public:
	X_API_MEMBER xLogger();
	X_API_MEMBER ~xLogger();

	virtual void SetLogLevel(eLogLevel ll)                = 0;
	virtual void Log(eLogLevel ll, const char * fmt, ...) = 0;

	template <typename... Args>
	X_INLINE void V(const char * fmt, Args &&... args) {
		Log(eLogLevel::Verbose, fmt, std::forward<Args>(args)...);
	}

#ifndef NDEBUG
	template <typename... Args>
	X_INLINE void D(const char * fmt, Args &&... args) {
		Log(eLogLevel::Debug, fmt, std::forward<Args>(args)...);
	}
#else
	template <typename... Args>
	X_INLINE void D(const char * fmt, Args &&... args) {
		Pass();
	}
#endif

	template <typename... Args>
	X_INLINE void I(const char * fmt, Args &&... args) {
		Log(eLogLevel::Info, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	X_INLINE void W(const char * fmt, Args &&... args) {
		Log(eLogLevel::Warning, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	X_INLINE void E(const char * fmt, Args &&... args) {
		Log(eLogLevel::Error, fmt, std::forward<Args>(args)...);
	}
};

class xNonLogger final : public xLogger {
	void SetLogLevel(eLogLevel ll) override {}
	void Log(eLogLevel ll, const char * fmt, ...) override {}
};

class xBaseLogger final : public xLogger {
public:
	X_API_MEMBER xBaseLogger();
	X_API_MEMBER ~xBaseLogger();

	X_API_MEMBER bool Init(const char * PathPtr = nullptr, bool AutoStdout = true);
	X_API_MEMBER void Clean();

	X_INLINE bool IsStdout() const { return _LogFile == stdout; }

	X_API_MEMBER void SetLogLevel(eLogLevel ll) override { _LogLevel = ll; }
	X_API_MEMBER void Log(eLogLevel ll, const char * fmt, ...) override;

	X_API_MEMBER FILE * Lock() {
		_SyncMutex.lock();
		if (!_LogFile) {
			_SyncMutex.unlock();
		}
		return _LogFile;
	}
	X_API_MEMBER void Unlock(FILE *&& ExpiringFilePtr) {
		assert(ExpiringFilePtr == _LogFile);
		_SyncMutex.unlock();
	}

private:
	std::filesystem::path  _LogFilename;
	std::mutex             _SyncMutex;
	std::atomic<eLogLevel> _LogLevel{ eLogLevel::Debug };
	FILE *                 _LogFile = nullptr;
};

class xMemoryLogger final : public xLogger {
public:
	X_API_MEMBER xMemoryLogger();
	X_API_MEMBER ~xMemoryLogger();

	X_API_MEMBER bool Init(size32_t MaxLineNumber = 10000, size32_t MaxLineSize = 1024);
	X_API_MEMBER void Clean();

	X_API_MEMBER void SetLogLevel(eLogLevel ll) override { _LogLevel = ll; }
	X_API_MEMBER void Log(eLogLevel ll, const char * fmt, ...) override;
	X_API_MEMBER void Output(FILE * fp = stdout);

private:
	xSpinlock              _Spinlock;
	std::atomic<eLogLevel> _LogLevel{ eLogLevel::Debug };

	size_t _LineSize     = 0;
	size_t _RealLineSize = 0;
	char * _LogBufferPtr = nullptr;

	size_t _LineNumber       = 0;
	size_t _CurrentLineIndex = 0;

	// Format:
	// Length@size32_t + Output + "\n\0"
	static constexpr const size_t ExtraSize          = 2 /* \n\0 */;
	static constexpr const size_t LineLeadBufferSize = 48;
};

X_END
