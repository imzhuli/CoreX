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

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct xX {
	int  i = 1024;
	xX * operator->() {
		cout << "operator" << endl;
		return this;
	}
};

int main(int argc, char ** argv) {
	Touch(IC, ICG);

	xX   X;
	auto PX = &X;
	cout << X->i << endl;
	cout << (*PX)->i << endl;

	return 0;
}
