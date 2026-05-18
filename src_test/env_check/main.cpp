#include <wsi/wsi.hpp>
#include <xengine/engine.hpp>

using namespace xel;

xWindowHandle   WindowPtr      = {};
xWindowSettings WindowSettings = {
	// .WindowMode = eWindowMode::FullScreen,
};

int main(int, char **) {

	auto InitOptions    = xEngineInitOptions();
	InitOptions.OnStart = [] { 
        WindowPtr = CreateWindow(WindowSettings);
        /* StopXEngine(); */ };
	InitOptions.OnStop  = [] {
        // if (WindowPtr) {
        DeferDestroyWindow(Steal(WindowPtr));
        // }
	};
	RunXEngine(InitOptions);
	return 0;
}