#pragma once
#pragma once
#include "./core_min.hpp"
#include "./core_value_util.hpp"

#include <random>

X_BEGIN

class xIndexId;
template <bool RandomKey = false>
class xIndexIdPool;
template <typename tValue, bool RandomKey = false>
class xIndexedStorage;

/***
 * @name xIndexId
 * @brief an 64 bit integer, with lower 32 bits representing an Index
 *        and the higher 32 bit with random value as a check key
 * @note
 *    The highest bit in Index part and is always zero, so that an index is always positive,
 *    The highest bit in Key part and is always zero, so that an Key is always positive,
 *    The second highest bit in Key part is always one (aka: KeyInUseBitmask == 0x4000'0000u),
 *    Since all allocated index has KeyInUseBitmask set to 1, a valid index id is never zero;
 * */
class xIndexId final {
public:
	X_INLINE xIndexId() = default;
	X_INLINE constexpr xIndexId(uint64_t Value)
		: _Value(Value){};

	X_INLINE constexpr          operator uint64_t() const { return _Value; }
	X_INLINE constexpr uint64_t operator()() const { return _Value; }

	X_INLINE uint32_t GetIndex() const { return static_cast<uint32_t>(_Value); }
	X_INLINE uint32_t GetKey() const { return static_cast<uint32_t>(_Value >> 32); }

private:
	uint64_t _Value = 0;

	template <bool RandomKey>
	friend class xIndexIdPool;
	template <typename tValue, bool RandomKey>
	friend class xIndexedStorage;

	static constexpr const uint32_t MaxIndexSize    = ((uint32_t)0x1FFF'FFFFu);  // MaxIndexSize & KeyInUseBitmask == 0
	static constexpr const uint32_t NoFreeIndex     = ((uint32_t)0x2000'0000u);  // NoFreeIndex & KeyInUseBitmask == 0
	static constexpr const uint32_t KeyInUseBitmask = ((uint32_t)0x4000'0000u);
	static constexpr const uint32_t KeyMask         = ((uint32_t)0x7FFF'FFFFu);  // ensure Id(Key) always positive

	X_STATIC_INLINE bool IsSafeKey(uint32_t Key) { return X_LIKELY(Key & KeyInUseBitmask); }

	X_API_STATIC_MEMBER uint32_t TimeSeed();
};
static_assert(sizeof(xIndexId) == sizeof(uint64_t) && alignof(xIndexId) == alignof(uint64_t));

template <bool RandomKey>
class xIndexIdPool final : xNonCopyable {
public:
	bool Init(size_t Size, uint32_t InitCounter = xIndexId::TimeSeed()) {
		assert(Size <= xIndexId::MaxIndexSize);
		assert(_IdPoolPtr == nullptr);
		assert(_NextFreeIdIndex == xIndexId::NoFreeIndex);
		assert(_InitedId == 0);

		_IdPoolPtr = new (std::nothrow) uint32_t[Size];
		if (!_IdPoolPtr) {
			return false;
		}
		_IdPoolSize      = (size32_t)Size;
		_InitedId        = 0;
		_NextFreeIdIndex = xIndexId::NoFreeIndex;

		_Counter = InitCounter;
		if constexpr (RandomKey) {
			_Random32.seed(_Counter);
		}
		return true;
	}

	void Clean() {
		assert(_IdPoolPtr);
		delete[] X_DEBUG_STEAL(_IdPoolPtr);
		X_DEBUG_RESET(_InitedId);
		X_DEBUG_RESET(_IdPoolSize);
		X_DEBUG_RESET(_NextFreeIdIndex, xIndexId::NoFreeIndex);
	}

	X_INLINE xIndexId Acquire() {
		uint32_t Index;
		if (_NextFreeIdIndex == xIndexId::NoFreeIndex) {
			if (_InitedId >= _IdPoolSize) {
				return {};
			}
			Index = _InitedId++;
		} else {
			Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex]);
		}
		uint32_t Rand     = RandomKey ? _Random32() : (_Counter += CounterStep);
		Rand             |= xIndexId::KeyInUseBitmask;
		Rand             &= xIndexId::KeyMask;
		_IdPoolPtr[Index] = Rand;
		return { (static_cast<uint64_t>(Rand) << 32) + Index };
	}

	X_INLINE void Release(const xIndexId & Id) {
		uint32_t Index    = Id.GetIndex();
		_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
	}

	X_INLINE bool Check(const xIndexId & Id) {
		uint32_t Index = Id.GetIndex();
		if (!X_LIKELY(Index < _IdPoolSize)) {
			return false;
		}
		auto Key = Id.GetKey();
		return X_LIKELY(xIndexId::IsSafeKey(Key)) && X_LIKELY(Key == _IdPoolPtr[Index]);
	}

	X_INLINE bool CheckAndRelease(const xIndexId & Id) {
		uint32_t Index = Id.GetIndex();
		if (!X_LIKELY(Index < _IdPoolSize)) {
			return false;
		}
		auto Key = Id.GetKey();
		if (!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == _IdPoolPtr[Index])) {
			return false;
		}
		_IdPoolPtr[Index] = Steal(_NextFreeIdIndex, Index);
		return true;
	}

private:
	size32_t                        _InitedId        = 0;
	size32_t                        _IdPoolSize      = 0;
	uint32_t *                      _IdPoolPtr       = nullptr;
	uint32_t                        _NextFreeIdIndex = xIndexId::NoFreeIndex;
	uint32_t                        _Counter         = 0;
	std::mt19937                    _Random32;
	static constexpr const uint32_t CounterStep = 1;
};

template <typename tValue, bool RandomKey>
class xIndexedStorage final : xNonCopyable {
	static_assert(!std::is_reference_v<tValue> && !std::is_const_v<tValue>);
	struct xNode {
		union {
			uint32_t NextFreeIdIndex;
			uint32_t Key;
		};
		xHolder<tValue> ValueHolder;
	};

public:
	X_INLINE bool Init(size_t Size, uint32_t InitCounter = xIndexId::TimeSeed()) {
		assert(Size <= xIndexId::MaxIndexSize);
		assert(_IdPoolPtr == nullptr);
		assert(_NextFreeIdIndex == xIndexId::NoFreeIndex);
		assert(_InitedId == 0);

		_IdPoolPtr = new (std::nothrow) xNode[Size];
		if (!_IdPoolPtr) {
			return false;
		}
		_IdPoolSize      = (size32_t)Size;
		_InitedId        = 0;
		_NextFreeIdIndex = xIndexId::NoFreeIndex;

		_Counter = InitCounter;
		if constexpr (RandomKey) {
			_Random32.seed(_Counter);
		}
		return true;
	}

	X_INLINE void Clean() {
		assert(_IdPoolPtr);
		if constexpr (!std::is_trivially_destructible_v<tValue>) {
			for (size_t Index = 0; Index < _InitedId; ++Index) {
				auto & Node = _IdPoolPtr[Index];
				if (!(Node.Key & xIndexId::KeyInUseBitmask)) {
					continue;
				}
				Node.ValueHolder.Destroy();
			}
		}
		delete[] Steal(_IdPoolPtr);
		_InitedId        = 0;
		_IdPoolSize      = 0;
		_NextFreeIdIndex = xIndexId::NoFreeIndex;
	}

	X_INLINE xIndexId Acquire() {
		uint32_t Index;
		xNode *  NodePtr;
		if (_NextFreeIdIndex == xIndexId::NoFreeIndex) {
			if (_InitedId >= _IdPoolSize) {
				return {};
			}
			NodePtr = &_IdPoolPtr[Index = _InitedId++];
		} else {
			NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
		}
		uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
		Rand         |= xIndexId::KeyInUseBitmask;
		Rand         &= xIndexId::KeyMask;
		NodePtr->Key  = Rand;
		try {
			NodePtr->ValueHolder.Create();
		} catch (...) {
			NodePtr->NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
			throw;
		}
		return { (static_cast<uint64_t>(Rand) << 32) + Index };
	}

	X_INLINE xIndexId Acquire(const tValue & Value) {
		uint32_t Index;
		xNode *  NodePtr;
		if (_NextFreeIdIndex == xIndexId::NoFreeIndex) {
			if (_InitedId >= _IdPoolSize) {
				return {};
			}
			NodePtr = &_IdPoolPtr[Index = _InitedId++];
		} else {
			NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
		}
		uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
		Rand         |= xIndexId::KeyInUseBitmask;
		Rand         &= xIndexId::KeyMask;
		NodePtr->Key  = Rand;
		try {
			NodePtr->ValueHolder.CreateValue(Value);
		} catch (...) {
			NodePtr->NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
			throw;
		}
		return { (static_cast<uint64_t>(Rand) << 32) + Index };
	}

	X_INLINE xIndexId Acquire(tValue && Value) {
		uint32_t Index;
		xNode *  NodePtr;
		if (_NextFreeIdIndex == xIndexId::NoFreeIndex) {
			if (_InitedId >= _IdPoolSize) {
				return {};
			}
			NodePtr = &_IdPoolPtr[Index = _InitedId++];
		} else {
			NodePtr = &_IdPoolPtr[Index = Steal(_NextFreeIdIndex, _IdPoolPtr[_NextFreeIdIndex].NextFreeIdIndex)];
		}
		uint32_t Rand = RandomKey ? _Random32() : (_Counter += CounterStep);
		Rand         |= xIndexId::KeyInUseBitmask;
		Rand         &= xIndexId::KeyMask;
		NodePtr->Key  = Rand;
		try {
			NodePtr->ValueHolder.CreateValue(std::move(Value));
		} catch (...) {
			NodePtr->NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
			throw;
		}
		return { (static_cast<uint64_t>(Rand) << 32) + Index };
	}

	X_INLINE void Release(const xIndexId & Id) {
		uint32_t Index       = Id.GetIndex();
		auto &   Node        = _IdPoolPtr[Index];
		Node.NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
		Node.ValueHolder.Destroy();
	}

	X_INLINE bool Check(const xIndexId & Id) const {
		uint32_t Index = Id.GetIndex();
		if (!X_LIKELY(Index < _IdPoolSize)) {
			return false;
		}
		auto   Key  = Id.GetKey();
		auto & Node = _IdPoolPtr[Index];
		return X_LIKELY(xIndexId::IsSafeKey(Key)) && X_LIKELY(Key == Node.Key);
	}

	X_INLINE tValue * CheckAndGet(const xIndexId & Id) {
		uint32_t Index = Id.GetIndex();
		if (!X_LIKELY(Index < _IdPoolSize)) {
			return nullptr;
		}
		auto   Key  = Id.GetKey();
		auto & Node = _IdPoolPtr[Index];
		if (!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == Node.Key)) {
			return nullptr;
		}
		return Node.ValueHolder.GetAddress();
	}

	X_INLINE const tValue * CheckAndGet(const xIndexId & Id) const {
		uint32_t Index = Id.GetIndex();
		if (!X_LIKELY(Index < _IdPoolSize)) {
			return nullptr;
		}
		auto   Key  = Id.GetKey();
		auto & Node = _IdPoolPtr[Index];
		if (!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == Node.Key)) {
			return nullptr;
		}
		return Node.ValueHolder.GetAddress();
	}

	X_INLINE bool CheckAndRelease(const xIndexId & Id) {
		uint32_t Index = Id.GetIndex();
		if (!X_LIKELY(Index < _IdPoolSize)) {
			return false;
		}
		auto   Key  = Id.GetKey();
		auto & Node = _IdPoolPtr[Index];
		if (!X_LIKELY(xIndexId::IsSafeKey(Key)) || !X_LIKELY(Key == Node.Key)) {
			return false;
		}
		Node.NextFreeIdIndex = Steal(_NextFreeIdIndex, Index);
		Node.ValueHolder.Destroy();
		return true;
	}

	X_INLINE tValue & operator[](const xIndexId & Id) const { return *_IdPoolPtr[Id.GetIndex()].ValueHolder; }

	X_INLINE xIndexId GetObjectId(const tValue * Reference) {
		auto HolderPtr = xHolder<tValue>::O2H(Reference);
		auto NodePtr   = X_Entry(HolderPtr, const xNode, ValueHolder);
		auto Index     = MakeUnsigned(NodePtr - _IdPoolPtr);
		if (Index >= _InitedId) {
			// X_DEBUG_PRINTF("overflow: Index=%zi, Inited=%zi", size_t(Index), size_t(_InitedId));
			return 0;
		}
		if (!xIndexId::IsSafeKey(NodePtr->Key)) {
			// X_DEBUG_PRINTF("invalid key: %" PRIx32 "", NodePtr->Key);
			return 0;
		}
		return { (static_cast<uint64_t>(NodePtr->Key) << 32) + Index };
	};

	static constexpr const size_t NodeSize = sizeof(xNode);

private:
	xNode *  _IdPoolPtr       = nullptr;
	size32_t _IdPoolSize      = 0;
	size32_t _InitedId        = 0;
	uint32_t _NextFreeIdIndex = xIndexId::NoFreeIndex;

	std::mt19937                    _Random32;
	uint32_t                        _Counter    = 0;
	static constexpr const uint32_t CounterStep = 1;
};

X_END
