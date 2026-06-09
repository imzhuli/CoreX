#include "./_.hpp"

X_SERVICE_BEGIN

xServerIdComponent ExtractServerIdComponent(uint64_t ServerId) {
	ServerId &= 0x0FFFFFFF'FFFFFFFFu;
	return {
		.Id		  = (uint32_t)(ServerId >> 32),
		.Random16 = (uint16_t)(ServerId >> 16),
		.Checksum = (uint16_t)(ServerId),
	};
}

xServerIdInternal ExtractServerIdInternalFromPureId(uint32_t Id) {
	return {
		.Type	  = (xServerType)(Id >> 19),
		.ObjectId = Id & 0x07FFFF,
	};
}

xServerType ExtractServerType(uint64_t ServerId) {
	assert(ServerId);
	return (xServerType)(ServerId >> 51);
}

uint32_t ExtractServerObjectId(uint64_t ServerId) {
	assert(ServerId);
	auto ObjectId = (ServerId >> 32) & 0x0007FFFF;
	return uint32_t(ObjectId);
}

xServerIdInternal ExtractServerIdInternal(uint64_t ServerId) {
	assert(ServerId);
	return ExtractServerIdInternalFromPureId(uint32_t(ServerId >> 32));
}

X_SERVICE_END
