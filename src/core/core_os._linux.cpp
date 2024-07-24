#include "./core_os.hpp"

#ifdef X_SYSTEM_LINUX

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

X_BEGIN

void Daemonize() {
	daemon(1, 1);
}

static void GuardProcess(pid_t ChildPid) {
	int status;
	int wpid;
	do {
		wpid = waitpid(ChildPid, &status, 0);  // 等待指定的子进程结束
	} while (wpid == -1 && errno == EINTR);
}

void RunAsGuard() {
	while (true) {
		pid_t pid = fork();
		if (pid < 0) {  // error
			X_PFATAL("failed to create sub thread");
			return;
		} else if (!pid) {  // child process
			return;
		} else {  // pid is Child's pid
			GuardProcess(pid);
		}
	}
}

X_END

#endif
