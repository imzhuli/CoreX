#include <condition_variable>
#include <core/core_min.hpp>
#include <core/string.hpp>
#include <network/udp_channel.hpp>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <server_arch/message.hpp>
#include <string>
#include <thread>

using namespace xel;
using namespace std;

auto IC = xIoContext();
auto ICG = xResourceGuard(IC);

int main(int argc, char ** argv) {

	auto Addr = xNetAddress::Parse("0.0.0.0", 0);

	auto U = xUdpChannel();
	auto UG = xResourceGuard(U, &IC, Addr, nullptr);

	cout << U.GetBindAddress().ToString() << endl;

	return 0;
}
