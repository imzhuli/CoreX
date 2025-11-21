#include "./client_wrapper.hpp"

X_BEGIN

bool xClientWrapper::Init(xIoContext * ICP) {
	this->ICP = ICP;
	return true;
}

bool xClientWrapper::Init(xIoContext * ICP, const xNetAddress & Address) {
	this->ICP = ICP;
	UpdateTarget(Address);
	return true;
}

void xClientWrapper::Clean() {
	if (Steal(HasInstance)) {
		xClient::Clean();
	}
	Reset(ICP);
}

void xClientWrapper::Tick(uint64_t NowMS) {
	if (!HasInstance) {
		return;
	}
	xClient::Tick(NowMS);
}

void xClientWrapper::Kill() {
	if (!HasInstance) {
		return;
	}
	xClient::Kill();
}

void xClientWrapper::UpdateTarget(const xNetAddress & Address) {
	if (Steal(HasInstance)) {
		xClient::Clean();
	}
	if (Address) {
		RuntimeAssert((HasInstance = xClient::Init(this->ICP, Address)));
	}
}

void xClientWrapper::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	if (!HasInstance) {
		return;
	}
	xClient::PostMessage(CmdId, RequestId, Message);
}

void xClientWrapper::OnServerConnected() {
	assert(HasInstance);
	OnConnected();
}

void xClientWrapper::OnServerClose() {
	assert(HasInstance);
	OnDisconnected();
}

bool xClientWrapper::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
	assert(HasInstance);
	return OnPacket(CommandId, RequestId, PayloadPtr, PayloadSize);
}

X_END