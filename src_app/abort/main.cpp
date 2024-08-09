#include <core/core_min.hpp>
using namespace xel;

/* this program is for testing platform behaviour on QuickExit (coredump or not, wait for thread termination and extra..)*/

int main(int, char **) {
	Fatal("Abort");
	return 0;
}
