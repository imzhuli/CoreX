#include <core/core_time.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/io_context.hpp>
#include <server_arch/client.hpp>

using namespace std;
using namespace xel;

xTimer Timer;
struct xHelloWorldClient : public xTcpConnection::iListener {
	void   OnConnected(xTcpConnection * CP) { cout << "Connected" << endl; }
	void   OnPeerClose(xTcpConnection * CP) { cout << "Closed" << endl; }
	void   OnFlush(xTcpConnection * CP) { cout << "Flush" << endl; }
	size_t OnData(xTcpConnection * CP, ubyte * DataPtr, size_t DataSize) {
		cout << HexShow(DataPtr, DataSize) << endl;
		return DataSize;
	}
};

static xIoContext        IoCtx;
static xTcpConnection    Conn;
static xHelloWorldClient Client;
static xNetAddress       ServerAddress = xNetAddress::Parse("192.168.5.112:3399");

int main(int argc, char ** argv) {
	auto IR = xResourceGuard(IoCtx);
	if (!IR) {
		cerr << "Invalid IR" << endl;
		return -1;
	}
	auto CR = xResourceGuard(Conn, &IoCtx, ServerAddress, &Client);
	if (!CR) {
		cerr << "Invalid CR" << endl;
		return -1;
	}

	while (true) {
		IoCtx.LoopOnce();
		if (Timer.TestAndTag(1s)) {
			Conn.SuspendReading();
			Conn.ResumeReading();
		}
	}
	return 0;
}
