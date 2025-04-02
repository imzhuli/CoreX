#include "./packet.hpp"

X_BEGIN

static_assert(std::is_standard_layout_v<xPacketHeader>);
static_assert(sizeof(xPacketHeader) == PacketHeaderSize);
static_assert(sizeof(xPacketHeader) == xPacketHeader::Size);

X_END
