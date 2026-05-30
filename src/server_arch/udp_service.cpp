#include "./udp_service.hpp"

#include "../core/string.hpp"

X_BEGIN

bool xUdpServiceChannelHandle::IsValid() const {
	return Service && RemoteAddress;
}

void xUdpServiceChannelHandle::PostData(const void * DataPtr, size_t DataSize) const {
	return Service->PostData(RemoteAddress, DataPtr, DataSize);
}

void xUdpServiceChannelHandle::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const {
	Service->PostMessage(RemoteAddress, CmdId, RequestId, Message);
}

void xUdpService::OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) {
	assert(this == ChannelPtr);
	if (DataSize < PacketHeaderSize) {
		return;
	}

	auto Header = xPacketHeader::Parse(DataPtr);
	if (!Header) { /* header error */
		return;
	}
	auto PacketSize = Header.PacketSize;
	if (DataSize != PacketSize) {
		return;
	}
	if (Header.IsInternalRequest()) {  // ignore all internal request
		return;
	}

	auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
	auto PayloadSize = Header.GetPayloadSize();
	OnPacket(
		{
			this,
			RemoteAddress,
		},
		Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize
	);
	return;
}

void xUdpService::PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	ubyte Buffer[MaxPacketSize];
	auto  PSize = WriteMessage(Buffer, CmdId, RequestId, Message);
	if (!PSize) {
		return;
	}
	PostData(RemoteAddress, Buffer, PSize);
}

X_END
