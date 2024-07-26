#include <core/memory_pool.hpp>
#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char ** argv) {
	auto Pool = xel::xFixedObjectPool<int>();
	auto PG   = xel::xResourceGuard(Pool, 2);
	RuntimeAssert(PG);

	auto PoolSS = xel::xFixedObjectPool<int>();
	auto PSG    = xel::xResourceGuard(PoolSS, 0);
	RuntimeAssert(PSG);

	auto P1 = Pool.Create();
	auto P2 = Pool.Create();
	auto P3 = Pool.Create();
	auto P4 = Pool.Create();

	cout << "P1: " << (void *)P1 << endl;
	cout << "P2: " << (void *)P2 << endl;
	cout << "P3: " << (void *)P3 << endl;
	cout << "P4: " << (void *)P4 << endl;

	Pool.Destroy(P1);
	Pool.Destroy(P2);
	Pool.Destroy(P3);
	Pool.Destroy(P4);

	P1 = Pool.Create();
	P2 = Pool.Create();
	P3 = Pool.Create();
	P4 = Pool.Create();

	cout << "P1: " << (void *)P1 << endl;
	cout << "P2: " << (void *)P2 << endl;
	cout << "P3: " << (void *)P3 << endl;
	cout << "P4: " << (void *)P4 << endl;

	Pool.Destroy(P1);
	Pool.Destroy(P2);
	Pool.Destroy(P3);
	Pool.Destroy(P4);

	auto PP = PoolSS.Create();
	cout << "PP: " << (void *)PP << endl;
	PoolSS.Destroy(PP);

	return 0;
}
