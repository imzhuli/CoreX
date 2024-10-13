#include "./core_os.hpp"

#ifdef X_SYSTEM_WINDOWS

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

X_BEGIN

void Daemonize() {
	// Step 1: Create a new process to run the daemonized code
	STARTUPINFO         si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb          = sizeof(si);
	si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	ZeroMemory(&pi, sizeof(pi));

	// Create the new process
	if (!CreateProcess(
			NULL,              // No module name (use command line)
			GetCommandLine(),  // Command line
			NULL,              // Process handle not inheritable
			NULL,              // Thread handle not inheritable
			TRUE,              // Set handle inheritance to TRUE
			CREATE_NO_WINDOW,  // No console window
			NULL,              // Use parent's environment block
			NULL,              // Use parent's starting directory
			&si,               // Pointer to STARTUPINFO structure
			&pi
		)  // Pointer to PROCESS_INFORMATION structure
	) {
		X_PFATAL("CreateProcess failed: %i", GetLastError());
		return;
	}

	// Close process and thread handles.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// Exit the parent process
	ExitProcess(0);
}

void RunAsGuard() {
	X_PERROR("RunAsGuard is not supported on windows platform");
}

X_END

#endif
