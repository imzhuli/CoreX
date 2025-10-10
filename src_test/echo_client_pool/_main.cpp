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

auto Pool = xClientPool();

auto PoolGuard = xResourceGuard(Pool, &IC, 100);

auto     TA_Client  = xNetAddress::Parse("127.0.0.1:10000");
auto     TA_Release = xNetAddress::Parse("127.0.0.1:12000");
uint64_t CID_Client;
uint64_t CID_Release;

void InitClientConn() {

	Pool.OnServerConnected = [](xClientConnection & CC) {
		X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", CC.GetConnectionId(), CC.GetTargetAddress().ToString().c_str());
		auto HW = xHello();
		auto PM = CC.PostMessage(1, 2, HW);
		X_RUNTIME_ASSERT(PM);
	};

	Pool.OnServerPacket = [](xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
		X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s, CommandId=%" PRIx32 "", CC.GetConnectionId(), CC.GetTargetAddress().ToString().c_str(), CommandId);
		return true;
	};

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