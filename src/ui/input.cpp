#include "./input.hpp"

X_BEGIN

bool xInputManager::Init() {
	return true;
}

void xInputManager::Clean() {
}

void xInputManager::DetectRawInputChanges() {
	RequireRawEventDetection = false;
	// TODO: detect key state changes
}

X_END
