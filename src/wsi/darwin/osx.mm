#include "./osx.h"

#ifdef X_SYSTEM_OSX
#include <Cocoa/Cocoa.h>

X_BEGIN

size_t GetMacMenuHeight() {
	return (size_t)[[[NSApplication sharedApplication] menu] menuBarHeight];
}

X_END
#endif
