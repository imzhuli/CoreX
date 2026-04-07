#include "./tcp_client_wrapper.hpp"

X_BEGIN

bool xTcpClientWrapper::Init(xIoContext * ICP) {
	this->ICP = ICP;
	return true;
}

bool xTcpClientWrapper::Init(xIoContext * ICP, const xNetAddress & Address) {
	this->ICP = ICP;
	UpdateTarget(Address);
	return true;
}

void xTcpClientWrapper::Clean() {
	if (Steal(HasInstance)) {
		xTcpClient::Clean();
	}
	Reset(ICP);
}

void xTcpClientWrapper::Tick(uint64_t NowMS) {
	if (!HasInstance) {
		return;
	}
	xTcpClient::Tick(NowMS);
}

void xTcpClientWrapper::Kill() {
	if (!HasInstance) {
		return;
	}
	xTcpClient::Kill();
}

void xTcpClientWrapper::UpdateTarget(const xNetAddress & Address) {
	if (Steal(HasInstance)) {
		xTcpClient::Clean();
	}
	if (Address) {
		RuntimeAssert((HasInstance = xTcpClient::Init(this->ICP, Address)));
	}
}

void xTcpClientWrapper::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	if (!HasInstance) {
		return;
	}
	xTcpClient::PostMessage(CmdId, RequestId, Message);
}

X_END