#include <wsi/wsi.hpp>
#include <xengine/engine.hpp>

using namespace xel;

iWindow *       WindowPtr      = nullptr;
xWindowSettings WindowSettings = {
	// .WindowMode = eWindowMode::FullScreen,
};

int main(int, char **) {
	auto InitOptions    = xEngineInitOptions();
	InitOptions.OnStart = [] { /* StopXEngine(); */ };
	// InitOptions.OnStop  = [] {
	//     if (WindowPtr) {
	//         DeferDestroyWindow(Steal(WindowPtr));
	//     }
	// };
	RunXEngine(InitOptions);
	return 0;
}