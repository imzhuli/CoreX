#include "./core_os.hpp"

#ifdef X_SYSTEM_DARWIN

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

X_BEGIN

void Daemonize() {
	X_RUNTIME_ASSERT(!daemon(1, 1));
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
			X_PERROR("failed to create sub process");
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		} else if (!pid) {  // child process
			return;
		} else {  // pid is Child's pid
			GuardProcess(pid);
		}
	}
	Unreachable();
}

X_END

#endif
