#pragma once
#include "../../core/core_value_util.hpp"
#include "../../object/object.hpp"
#include "../base/_.hpp"

#include <random>

X_SERVICE_BEGIN

struct xServerIdInternal {
	xServerType Type;
	uint32_t	ObjectId;
};

xServerType		  ExtractServerType(uint64_t ServerId);
uint32_t		  ExtractServerObjectId(uint64_t ServerId);
xServerIdInternal ExtractServerIdInternal(uint64_t ServerId);

class xServerIdManager final {
public:
	bool Init();
	void Clean();

	uint64_t AcquireServerId(xServerType Type = 0);
	uint64_t RegainServerId(uint64_t ServerId);
	bool	 ReleaseServerId(uint64_t ServerId);

	static constexpr size_t MaxServerIdCount() { return decltype(IdManager)::MaxObjectId; }

private:
	uint16_t GenerateRandom();
	uint16_t GenerateCheckSum(uint32_t IdIndex, uint16_t IdRandom);

private:
	xObjectIdManager IdManager;
	uint16_t *		 RandomPool = nullptr;

	xHolder<std::mt19937> RandomGeneratorHolder;
};

X_SERVICE_END
