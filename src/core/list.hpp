#pragma once
#include "./core_min.hpp"

X_BEGIN

// xList<tNode> type, with tNode extends xListNode
template <typename tNode>
class xList;

class xListNode {
private:
	xListNode * pPrev;
	xListNode * pNext;

	template <typename tNode>
	friend class xList;

public:
	X_INLINE xListNode() noexcept { ResetUnsafe(); }
	X_INLINE ~xListNode() noexcept { DetachUnsafe(); }

protected:
	X_INLINE             xListNode(const xListNode & Other) noexcept { ResetUnsafe(); }
	X_INLINE xListNode & operator=(const xListNode & Other) noexcept { return *this; }
	X_INLINE xListNode & operator=(xListNode && Other) = delete;

public:
	X_STATIC_INLINE bool IsLinked(const xListNode & Node) { return Node.pPrev != &Node; }
	X_STATIC_INLINE void Unlink(xListNode & Node) { return Node.Detach(); }

private:
	X_INLINE void ResetUnsafe() { pPrev = pNext = this; }

	X_INLINE void Detach() {
		DetachUnsafe();
		ResetUnsafe();
	}

	X_INLINE void TakePlaceOf(xListNode & other) {
		assert(pPrev == this && pNext == this);
		assert(IsLinked(other));
		TakePlaceOfUnsafe(other);
	}

	X_INLINE void AppendTo(xListNode & prev_node) {
		xListNode & next_node = *prev_node.pNext;
		prev_node.pNext       = this;
		next_node.pPrev       = this;
		pPrev                 = &prev_node;
		pNext                 = &next_node;
	}

	X_INLINE void DetachUnsafe() {
		pPrev->pNext = pNext;
		pNext->pPrev = pPrev;
	}

	X_INLINE void TakePlaceOfUnsafe(xListNode & other) {
		pPrev        = other.pPrev;
		pNext        = other.pNext;
		pNext->pPrev = this;
		pPrev->pNext = this;
		other.ResetUnsafe();
	}
};

template <typename tNode = xListNode>
class xList {
private:
	static_assert(std::is_base_of_v<xListNode, tNode>);
	static_assert(!std::is_reference_v<tNode> && !std::is_const_v<tNode>);
	xListNode _Head;

public:
	xList()              = default;
	xList(const xList &) = delete;
	X_INLINE xList(xList && other) { GrabListTail(other); }
	X_INLINE ~xList() { assert(IsEmpty()); }

public:
	X_INLINE void ResetUnsafe() { _Head.ResetUnsafe(); }
	X_INLINE bool IsEmpty() const { return _Head.pNext == &_Head; }
	X_INLINE void AddHead(tNode & rTarget) { static_cast<xListNode &>(rTarget).AppendTo(_Head); }
	X_INLINE void AddTail(tNode & rTarget) { static_cast<xListNode &>(rTarget).AppendTo(*_Head.pPrev); }
	X_INLINE void GrabHead(tNode & rTarget) {
		static_cast<xListNode &>(rTarget).DetachUnsafe();
		AddHead(rTarget);
	}
	X_INLINE void GrabTail(tNode & rTarget) {
		static_cast<xListNode &>(rTarget).DetachUnsafe();
		AddTail(rTarget);
	}
	X_INLINE void GrabListHead(xList & other) {
		if (other.IsEmpty()) {
			return;
		}
		xListNode * remoteHead = other._Head.pNext;
		xListNode * remoteTail = other._Head.pPrev;
		other._Head.ResetUnsafe();

		xListNode * localHead = _Head.pNext;
		_Head.pNext           = remoteHead;
		remoteHead->pPrev     = &_Head;
		localHead->pPrev      = remoteTail;
		remoteTail->pNext     = localHead;
	}
	X_INLINE void GrabListTail(xList & other) {
		if (other.IsEmpty()) {
			return;
		}
		xListNode * remoteHead = other._Head.pNext;
		xListNode * remoteTail = other._Head.pPrev;
		other._Head.ResetUnsafe();

		xListNode * localTail = _Head.pPrev;
		_Head.pPrev           = remoteTail;
		remoteTail->pNext     = &_Head;
		localTail->pNext      = remoteHead;
		remoteHead->pPrev     = localTail;
	}
	X_INLINE tNode * Head() {
		if (IsEmpty()) {
			return nullptr;
		}
		return &static_cast<tNode &>(*_Head.pNext);
	}
	X_INLINE tNode * Tail() {
		if (IsEmpty()) {
			return nullptr;
		}
		return &static_cast<tNode &>(*_Head.pPrev);
	}
	X_INLINE tNode * PopHead() {
		if (IsEmpty()) {
			return nullptr;
		}
		auto ret = _Head.pNext;
		ret->Detach();
		return &static_cast<tNode &>(*ret);
	}
	template <typename tCond>
	X_INLINE tNode * PopHead(tCond && C) {
		if (IsEmpty()) {
			return nullptr;
		}
		auto   ret   = _Head.pNext;
		auto & Node  = static_cast<tNode &>(*ret);
		auto & CNode = static_cast<const tNode &>(Node);
		if (!std::forward<tCond>(C)(CNode)) {
			return nullptr;
		}
		ret->Detach();
		return &Node;
	}
	X_INLINE tNode * PopTail() {
		if (IsEmpty()) {
			return nullptr;
		}
		auto ret = _Head.pPrev;
		ret->Detach();
		return &static_cast<tNode &>(*ret);
	}
	template <typename tCond>
	X_INLINE tNode * PopTail(tCond && C) {
		if (IsEmpty()) {
			return nullptr;
		}
		auto   ret   = _Head.pPrev;
		auto & Node  = static_cast<tNode &>(*ret);
		auto & CNode = static_cast<const tNode &>(Node);
		if (!std::forward<tCond>(C)(CNode)) {
			return nullptr;
		}
		ret->Detach();
		return &Node;
	}
	template <typename tProc>
	X_INLINE void ForEach(const tProc & P) const {
		for (auto NP = _Head.pNext; NP != &_Head; NP = NP->pNext) {
			P(static_cast<tNode &>(*NP));
		}
	}

	X_STATIC_INLINE void InsertBefore(tNode & Node, tNode & InsertPoint) { Node.AppendTo(*InsertPoint.pPrev); }
	X_STATIC_INLINE void InsertAfter(tNode & Node, tNode & InsertPoint) { Node.AppendTo(InsertPoint); }

	X_STATIC_INLINE void Remove(tNode & Node) { Node.Detach(); }
};

X_END
