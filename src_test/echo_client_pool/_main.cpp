#include <server_arch/client_pool.hpp>

using namespace std;
using namespace xel;

auto IC  = xIoContext();
auto ICG = xResourceGuard(IC);

struct xHello : public xBinaryMessage {

	void SerializeMembers() { W(HW); }
	void DeserializeMembers() { R(HW); }

	std::string HW = "hello world!";
};

struct xMyPool : public xClientPool {

	void OnServerConnected(xClientConnection & CC) override {
		xClientPool::OnServerConnected(CC);
		auto HW = xHello();
		auto PM = PostMessage(CC, 1, 2, HW);
		X_RUNTIME_ASSERT(PM);
	}

	bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override {
		xClientPool::OnServerPacket(CC, CommandId, RequestId, PayloadPtr, PayloadSize);
		X_DEBUG_PRINTF("?????????????????");
		return true;
	}
};
auto Pool = xMyPool();

auto PoolGuard = xResourceGuard(Pool, &IC, 100);

auto     TA_Client  = xNetAddress::Parse("127.0.0.1:10000");
auto     TA_Release = xNetAddress::Parse("127.0.0.1:12000");
uint64_t CID_Client;
uint64_t CID_Release;

void InitClientConn() {
	CID_Release = Pool.AddServer(TA_Client);
	X_RUNTIME_ASSERT(CID_Release);
}
//

void InitReleaseConn() {
	CID_Release = Pool.AddServer(TA_Release);
	X_RUNTIME_ASSERT(CID_Release);
}

void ReleaseChecker() {
	static bool Processed = false;
	static auto Timer     = xTimer();
	if (Processed) {
		return;
	}
	if (Timer.Elapsed() <= std::chrono::seconds(5)) {
		return;
	}
	Processed = true;

	//
	X_DEBUG_PRINTF("");
	Pool.RemoveServer(CID_Release);
}

int main(int, char **) {
	X_RUNTIME_ASSERT(PoolGuard);

	InitClientConn();
	// InitReleaseConn();

	while (true) {
		IC.LoopOnce();
		Pool.Tick();

		// ReleaseChecker();
	}

	return 0;
}