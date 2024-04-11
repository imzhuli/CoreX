#include <condition_variable>
#include <core/core_min.hpp>
#include <core/string.hpp>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace xel;
using namespace std;

static auto M = std::mutex();
static auto C = std::condition_variable();

void Foo() {
	auto G = std::unique_lock(M);
	std::abort();
}

int main(int argc, char ** argv) {
	auto G = std::unique_lock(M);
	auto T = std::thread(Foo);
	C.wait(G, [] { return false; });

	T.join();
	return 0;
}
