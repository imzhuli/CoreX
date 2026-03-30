#include <core/core_value_util.hpp>
#include <core/memory_pool.hpp>
#include <iostream>
#include <vector>

using namespace xel;
using namespace std;

int main(int argc, char ** argv) {

	auto P = xMemoryPool<int>();
	P.Init({
		.InitSize    = 10,
		.Addend      = 24,
		.MaxPoolSize = 66,
	});

	int * PA[1024] = {};

	for (size_t i = 0; i < Length(PA); ++i) {
		PA[i] = P.CreateValue(i);
	}
	size_t S = 0;
	for (auto & R : PA) {
		if (!R) {
			break;
		}
		cout << *R << endl;
		++S;
	}
	cout << "total: " << S << endl;

	P.Clean();

	return 0;
}
