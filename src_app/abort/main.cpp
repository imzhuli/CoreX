#include <core/core_min.hpp>
#include <iostream>

using namespace xel;
using namespace std;

/* this program is for testing platform behaviour on QuickExit (coredump or not, wait for thread termination and extra..)*/

struct X : xel::xNonCopyable {};

using xPF = void (*)(int, X &, X &&);

template <typename... T>
void IgnoreTT(T...) {
}

template <auto R, typename... T>
auto IgnoreTT(T...) {
	return R;
}

int main(int, char **) {

	auto x = X();
	Touch(x);

	int i = 0;
	IgnoreTT(i);

	xPF PF = IgnoreTT;
	Touch(PF);

	cout << IgnoreTT<true>() << endl;

	Fatal("Abort");
	return 0;
}
