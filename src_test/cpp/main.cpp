#include <condition_variable>
#include <core/core_min.hpp>
#include <core/string.hpp>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <network/udp_channel.hpp>
#include <server_arch/message.hpp>
#include <string>
#include <thread>

using namespace xel;
using namespace std;

struct xS : xListNode {
	int i = 0;
};

int main(int argc, char ** argv) {

	xList<xS> L;

	xS S0 = xS();
	xS S1 = xS();
	xS S2 = xS();
	xS S3 = xS();

	S0.i = 0;
	S1.i = 1;
	S2.i = 2;
	S3.i = 3;

	L.AddTail(S0);
	L.AddTail(S1);
	L.AddTail(S2);
	L.AddTail(S3);

	while (auto NP = L.PopHead([](const xS & S) { return S.i < 10; })) {
		cout << NP->i << endl;
	}

	return 0;
}
