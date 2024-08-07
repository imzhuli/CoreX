#pragma once
#include "./core_min.h"

#include <assert.h>

X_CNAME_BEGIN

/* Node */
typedef struct XelRBNode XelRBNode;
struct XelRBNode {
	XelRBNode * ParentPtr;
	XelRBNode * LeftNodePtr;
	XelRBNode * RightNodePtr;
	bool        RedFlag;
};

typedef struct XelRBInsertSlot {
	XelRBNode *  InsertParentPtr;
	XelRBNode ** InsertSubNodeRefPtr;
	XelRBNode *  FoundNodePtr;
} XelRBInsertSlot;

X_STATIC_INLINE void XRBN_Init(XelRBNode * NodePtr) {
	XelRBNode InitValue = { NULL, NULL, NULL, false };
	*NodePtr            = InitValue;
}

X_STATIC_INLINE bool XRBN_IsRoot(XelRBNode * NodePtr) {
	return !NodePtr->ParentPtr;
}
X_STATIC_INLINE bool XRBN_IsLeaf(XelRBNode * NodePtr) {
	return !NodePtr->LeftNodePtr && !NodePtr->RightNodePtr;
}
X_STATIC_INLINE bool XRBN_IsRed(XelRBNode * NodePtr) {
	return NodePtr->RedFlag;
}
X_STATIC_INLINE bool XRBN_IsGenericRed(XelRBNode * NodePtr) {
	return NodePtr && (NodePtr->RedFlag);
}
X_STATIC_INLINE bool XRBN_IsBlack(XelRBNode * NodePtr) {
	return !XRBN_IsRed(NodePtr);
}
X_STATIC_INLINE bool XRBN_IsGenericBlack(XelRBNode * NodePtr) {
	return !NodePtr || !XRBN_IsRed(NodePtr);
}
X_STATIC_INLINE void XRBN_MarkRed(XelRBNode * NodePtr) {
	NodePtr->RedFlag = true;
}
X_STATIC_INLINE void XRBN_MarkBlack(XelRBNode * NodePtr) {
	NodePtr->RedFlag = false;
}

// X_STATIC_INLINE void* XRBN_Cast(XelRBNode* NodePtr, size_t NodeMemberOffset) {
//     if (!NodePtr) {
//         return NULL;
//     }
//     return (void*)((unsigned char*)NodePtr - NodeMemberOffset);
// }
// #define XRBN_ENTRY(_What, Type, Member) ((Type*)(XRBN_Cast((_What), offsetof(Type, Member))))

X_STATIC_INLINE XelRBNode * XRBN_LeftMost(XelRBNode * NodePtr) {
	// assert(NodePtr);
	while (NodePtr->LeftNodePtr) {
		NodePtr = NodePtr->LeftNodePtr;
	}
	return NodePtr;
}

X_STATIC_INLINE XelRBNode * XRBN_RightMost(XelRBNode * NodePtr) {
	// assert(NodePtr);
	while (NodePtr->RightNodePtr) {
		NodePtr = NodePtr->RightNodePtr;
	}
	return NodePtr;
}

X_STATIC_INLINE XelRBNode * XRBN_Prev(XelRBNode * NodePtr) {
	XelRBNode * ParentPtr;
	if (NodePtr->LeftNodePtr) {
		return XRBN_RightMost(NodePtr->LeftNodePtr);
	}
	while ((ParentPtr = NodePtr->ParentPtr) && (NodePtr == ParentPtr->LeftNodePtr)) {
		NodePtr = ParentPtr;
	}
	return ParentPtr;
}

X_STATIC_INLINE XelRBNode * XRBN_Next(XelRBNode * NodePtr) {
	XelRBNode * ParentPtr;
	if (NodePtr->RightNodePtr) {
		return XRBN_LeftMost(NodePtr->RightNodePtr);
	}
	while ((ParentPtr = NodePtr->ParentPtr) && (NodePtr == ParentPtr->RightNodePtr)) {
		NodePtr = ParentPtr;
	}
	return ParentPtr;
}

/* Tree */
typedef struct XelRBTree XelRBTree;
struct XelRBTree {
	XelRBNode * RootPtr;
};

typedef int XRBT_KeyCompare(XelRBTree * TreePtr, const void * KeyPtr, XelRBNode * NodePtr);

X_STATIC_INLINE void XRBT_Init(XelRBTree * TreePtr) {
	XelRBTree InitValue = { NULL };
	*TreePtr            = InitValue;
}

X_STATIC_INLINE bool XRBT_IsEmpty(XelRBTree * TreePtr) {
	return !TreePtr->RootPtr;
}

X_STATIC_INLINE void XRBT_TrivialClean(XelRBTree * TreePtr) {
}

X_STATIC_INLINE XelRBNode * XRBT_First(XelRBTree * TreePtr) {
	return TreePtr->RootPtr ? XRBN_LeftMost(TreePtr->RootPtr) : NULL;
}

X_STATIC_INLINE XelRBNode * XRBT_Last(XelRBTree * TreePtr) {
	return TreePtr->RootPtr ? XRBN_RightMost(TreePtr->RootPtr) : NULL;
}

X_STATIC_INLINE XelRBNode * XRBT_Find(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr) {
	XelRBNode * CurrNodePtr = TreePtr->RootPtr;
	while (CurrNodePtr) {
		int CompareResult = (*CompFunc)(TreePtr, KeyPtr, CurrNodePtr);
		if (CompareResult < 0) {
			CurrNodePtr = CurrNodePtr->LeftNodePtr;
		} else if (CompareResult > 0) {
			CurrNodePtr = CurrNodePtr->RightNodePtr;
		} else {
			return CurrNodePtr;
		}
	}
	return NULL;
}

X_STATIC_INLINE XelRBInsertSlot XRBT_FindInsertSlot(XelRBTree * TreePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr) {
	XelRBInsertSlot InsertNode     = { NULL, NULL, NULL };
	XelRBNode **    CurrNodeRefPtr = &TreePtr->RootPtr;
	while (*CurrNodeRefPtr) {
		InsertNode.InsertParentPtr = *CurrNodeRefPtr;
		int CompareResult          = (*CompFunc)(TreePtr, KeyPtr, *CurrNodeRefPtr);
		if (CompareResult < 0) {
			CurrNodeRefPtr = &(*CurrNodeRefPtr)->LeftNodePtr;
		} else if (CompareResult > 0) {
			CurrNodeRefPtr = &(*CurrNodeRefPtr)->RightNodePtr;
		} else {
			InsertNode.FoundNodePtr = InsertNode.InsertParentPtr;
			CurrNodeRefPtr          = NULL;
			break;
		}
	}
	InsertNode.InsertSubNodeRefPtr = CurrNodeRefPtr;
	return InsertNode;
}

/**
 * @brief Replace a node, an old one MUST exist
 *
 * @return X_STATIC_INLINE
 */
X_STATIC_INLINE void XRBT_Replace(XelRBTree * TreePtr, XelRBInsertSlot InsertSlot, XelRBNode * NodePtr) {
	assert(!InsertSlot.InsertParentPtr && !InsertSlot.InsertSubNodeRefPtr && InsertSlot.InsertParentPtr == InsertSlot.FoundNodePtr);

	XelRBNode * ReplaceNodePtr = InsertSlot.InsertParentPtr;
	if ((NodePtr->LeftNodePtr = ReplaceNodePtr->LeftNodePtr)) {
		NodePtr->LeftNodePtr->ParentPtr = NodePtr;
	}
	if ((NodePtr->RightNodePtr = ReplaceNodePtr->RightNodePtr)) {
		NodePtr->RightNodePtr->ParentPtr = NodePtr;
	}
	NodePtr->RedFlag = ReplaceNodePtr->RedFlag;

	if ((NodePtr->ParentPtr = ReplaceNodePtr->ParentPtr)) {
		XelRBNode * ParentNodePtr = NodePtr->ParentPtr;
		if (ParentNodePtr->LeftNodePtr == ReplaceNodePtr) {
			ParentNodePtr->LeftNodePtr = NodePtr;
		} else {
			ParentNodePtr->RightNodePtr = NodePtr;
		}
	} else {  // root
		TreePtr->RootPtr = NodePtr;
	}
	XRBN_Init(ReplaceNodePtr);
	return;
}

#define XRBT_FOR_EACH(_iter, _tree) for (XelRBNode * _iter = XRBT_First((_tree)); _iter; _iter = XRBN_Next(_iter))

#define XRBT_FOR_EACH_SAFE(_iter, _tree) for (XelRBNode * _iter = XRBT_First((_tree)), *_safe = XRBN_Next(_iter); _iter; _iter = _safe, _safe = XRBN_Next(_iter))

/**
 * @brief Insert a new node at a insert slot
 * @return
 */
X_API void XRBT_Insert(XelRBTree * TreePtr, XelRBInsertSlot InsertSlot, XelRBNode * NodePtr);
/**
 * @brief Insert a node or replace an old one which matches the key, if that exists
 * @return Pointer to the orignal node, if it exitst. NULL, if an insertion takes place.
 */
X_API XelRBNode * XRBT_InsertOrReplace(XelRBTree * TreePtr, XelRBNode * NodePtr, XRBT_KeyCompare * CompFunc, const void * KeyPtr);
X_API void        XRBT_Remove(XelRBTree * TreePtr, XelRBNode * NodePtr);

// check if the tree is a valid rbtree
// X_API bool        XRBT_Check(XelRBTree * TreePtr);

X_CNAME_END
