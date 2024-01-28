#include <wsi/wsi.hpp>
#include <xengine/engine.hpp>

using namespace xel;

static iWindow *       WindowPtr      = nullptr;
static xWindowSettings WindowSettings = {
	// .WindowMode = eWindowMode::FullScreen,
};

int main(int, char **) {
	RunXEngine({ [] { WindowPtr = CreateWindow(WindowSettings); },
				 [] {
					 if (WindowPtr) {
						 DestroyWindow(Steal(WindowPtr));
					 }
				 } });
	return 0;
}