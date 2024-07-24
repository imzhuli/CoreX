#include "./core_os.hpp"

#ifdef X_SYSTEM_LINUX

#include <unistd.h>

void Daemonize() {
	daemon(1, 1);
}

#endif
