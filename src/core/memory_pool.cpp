#include "./memory_pool.hpp"

X_BEGIN

namespace __memory_pool__ {

	bool xFixedObjectPoolBase::CreateNodePool(size_t NodeSize, size_t PoolSize) {
		if (!(Pool = (unsigned char *)::malloc(NodeSize * PoolSize))) {
			return false;
		}
		this->NodeSize     = NodeSize;
		this->PoolSize     = PoolSize;
		this->InitedSize   = 0;
		this->NextFreeNode = nullptr;
		return true;
	}

	void xFixedObjectPoolBase::DestroyNodePool() {
		assert(Pool);
		::free(Pool);
		Pool = nullptr;
	}

	void * xFixedObjectPoolBase::Alloc() {
		if (auto OP = NextFreeNode) {  // there is pooled object
			NextFreeNode = OP->NextFreeNode;
			OP->IsPooled = true;
			return OP;
		}
		if (InitedSize < PoolSize) {
			auto OP      = reinterpret_cast<xPaddingNode *>(Pool + NodeSize * InitedSize++);  // first uninited object
			OP->IsPooled = true;
			return OP;
		}
		auto P = ::malloc(NodeSize);
		if (!P) {
			return nullptr;
		}
		static_cast<xPaddingNode *>(P)->IsPooled = false;
		return P;
	}

	void xFixedObjectPoolBase::Free(void * P) {
		auto NP = (xPaddingNode *)P;
		if (!NP->IsPooled) {
			::free(P);
			return;
		}
		// return to pool:
		NP->NextFreeNode = NextFreeNode;
		NextFreeNode     = NP;
		return;
	}

}  // namespace __memory_pool__

X_END
