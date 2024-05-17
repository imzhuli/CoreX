#include <core/core_min.hpp>
#include <core/core_time.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/io_context.hpp>
#include <network/tcp_connection.hpp>
#include <network/udp_channel.hpp>
#include <thread>

using namespace xel;
using namespace std;

static auto IC = xIoContext();
static auto TC = xTcpConnection();

struct xObserver : xTcpConnection::iListener {  // clang-format off
	size_t OnData(xTcpConnection * CP, void * DataPtr, size_t DataSize) {
		printf("OnData: from %s\n%s\n", CP->GetRemoteAddress().ToString().c_str(), HexShow(DataPtr, DataSize).c_str());
		return DataSize;
	}
};  // clang-format on
static auto OB = xObserver();

int main(int argc, char ** argv) {

	X_DEBUG_PRINTF("TEST_NDEBUG");

	auto ICG = xResourceGuard(IC);
	auto TA  = xNetAddress::Parse("183.2.172.185:80");
	auto BA  = xNetAddress::Make4();
	auto TCG = xResourceGuard(TC, &IC, TA, BA, &OB);

	const char * Get = "GET / HTTP/1.1\r\n\r\n";
	TC.PostData(Get, strlen(Get));

	// std::this_thread::sleep_for(xSeconds(10));
	auto T = xTimer();
	while (true) {
		if (T.TestAndTag(xSeconds(5))) {
			break;
		}
		IC.LoopOnce();
	}

	return 0;
}
