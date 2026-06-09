#pragma once
#include "../../core/core_value_util.hpp"
#include "../../object/object.hpp"
#include "./_.hpp"

#include <random>

X_SERVICE_BEGIN

class xServerIdManager final {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();

	X_API_MEMBER uint64_t AcquireServerId(xServerType Type = 0);
	X_API_MEMBER uint64_t RegainServerId(uint64_t ServerId);
	X_API_MEMBER bool	  ReleaseServerId(uint64_t ServerId);

	X_API_STATIC_MEMBER constexpr size_t MaxServerIdCount() { return decltype(IdManager)::MaxObjectId; }

private:
	X_PRIVATE_MEMBER uint16_t GenerateRandom();
	X_PRIVATE_MEMBER uint16_t GenerateCheckSum(uint32_t IdIndex, uint16_t IdRandom);

private:
	xObjectIdManager IdManager;
	uint16_t *		 RandomPool = nullptr;

	xHolder<std::mt19937> RandomGeneratorHolder;
};

X_SERVICE_END
