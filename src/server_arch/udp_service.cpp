#include "./udp_service.hpp"

#include "../core/string.hpp"

X_BEGIN

void xUdpServiceChannelHandle::PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
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
	auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
	if (DataSize != PacketSize) {         // wait for data
		return;
	}

	if (Header.IsRequestKeepAlive()) {
		ChannelPtr->PostKeepAlive(RemoteAddress);
		return;
	}

	auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
	auto PayloadSize = Header.GetPayloadSize();
	OnPacketCallback({ this }, RemoteAddress, Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize);
	return;
}

void xUdpService::PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	ubyte Buffer[MaxPacketSize];
	auto  PSize = WriteMessage(Buffer, CmdId, RequestId, Message);
	if (!PSize) {
		return;
	}
	PostData(Buffer, PSize, RemoteAddress);
}

X_END
