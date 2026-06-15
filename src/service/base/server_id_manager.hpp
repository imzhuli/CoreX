#pragma once
#include "../../core/core_value_util.hpp"
#include "../../object/object.hpp"
#include "./def.hpp"

#include <random>

X_SERVICE_BEGIN

class xServerIdManager final {

public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();

	X_API_MEMBER uint64_t AcquireServerId(xServerGroup Type = 0);
	X_API_MEMBER uint64_t RegainServerId(uint64_t ServerId);
	X_API_MEMBER bool	  ReleaseServerId(uint64_t ServerId);

private:
	X_PRIVATE_MEMBER uint16_t GenerateRandom();
	X_PRIVATE_MEMBER uint16_t GenerateCheckSum(uint32_t IdIndex, uint16_t IdRandom);

private:
	xObjectIdManager IdManager;
	uint16_t *		 RandomPool = nullptr;

	xHolder<std::mt19937> RandomGeneratorHolder;

public:
	static constexpr size_t MAX_SERVER_ID_COUNT = decltype(IdManager)::MaxObjectId;
	static constexpr size_t MAX_SERVER_ID_INDEX = MAX_SERVER_ID_COUNT - 1;
};

X_SERVICE_END
