#include <core/memory_pool.hpp>
#include <iostream>
using std::cout;
using std::endl;

struct xMM {
	xMM() { throw 1024; };
};

int main(int argc, char ** argv) {
	auto Pool = xel::xFixedObjectPool<int>();
	auto PG   = xel::xResourceGuard(Pool, 2);
	X_RUNTIME_ASSERT(PG);

	auto PoolSS = xel::xFixedObjectPool<xMM>();
	auto PSG    = xel::xResourceGuard(PoolSS, 0);
	X_RUNTIME_ASSERT(PSG);

	auto P1 = Pool.CreateValueWithList(1024);
	auto P2 = Pool.CreateValueWithList(1024);
	auto P3 = Pool.CreateValueWithList(1024);
	auto P4 = Pool.CreateValueWithList(1024);

	cout << "P1: " << (void *)P1 << ", " << *P1 << endl;
	cout << "P2: " << (void *)P2 << ", " << *P2 << endl;
	cout << "P3: " << (void *)P3 << ", " << *P3 << endl;
	cout << "P4: " << (void *)P4 << ", " << *P4 << endl;

	Pool.Destroy(P1);
	Pool.Destroy(P2);
	Pool.Destroy(P3);
	Pool.Destroy(P4);

	P1 = Pool.CreateValueInPool(1);
	P2 = Pool.CreateValueInPool(2);
	P3 = Pool.CreateValueInPool(3);
	P4 = Pool.CreateValueInPool(4);

	cout << "P1: " << (void *)P1 << ", " << *P1 << endl;
	cout << "P2: " << (void *)P2 << ", " << *P2 << endl;
	cout << "P3: " << (void *)P3 << endl;
	cout << "P4: " << (void *)P4 << endl;

	Pool.Destroy(P1);
	Pool.Destroy(P2);

	auto PP = PoolSS.Create();
	cout << "PP: " << (void *)PP << endl;
	X_RUNTIME_ASSERT(!PP);

	return 0;
}
