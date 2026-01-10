#include <wsi/wsi.hpp>
#include <xengine/engine.hpp>

using namespace xel;

static iWindow *       WindowPtr      = nullptr;
static xWindowSettings WindowSettings = {
	// .WindowMode = eWindowMode::FullScreen,
};

int main(int, char **) {
	auto InitOptions    = XEngineInitOptions();
	InitOptions.OnStart = [] { WindowPtr = CreateWindow(WindowSettings); };
	InitOptions.OnStop  = [] {
        if (WindowPtr) {
            DeferDestroyWindow(Steal(WindowPtr));
        }
	};
	RunXEngine(InitOptions);
	return 0;
}