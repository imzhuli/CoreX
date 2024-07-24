#include <chrono>
#include <core/core_os.hpp>
#include <iostream>
#include <thread>
using namespace std;

static void SubProcess() {
	cout << "new SubProcess" << endl;
	size_t Counter = 0;
	while (true) {
		cout << "SubProcess counter=" << ++Counter << endl;
		this_thread::sleep_for(chrono::seconds(1));
	}
}

int main(int, char **) {
	xel::RunAsGuard();
	SubProcess();
	return 0;
}
