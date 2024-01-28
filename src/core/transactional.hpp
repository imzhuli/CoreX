#pragma once
#include "./core_min.hpp"

X_BEGIN

class xTransactional : xNonCopyable {
public:
	X_API_MEMBER virtual bool Exec() {
		return true;
	}
	X_API_MEMBER virtual void Undo() {
	}

	X_STATIC_INLINE bool BatchExec(xTransactional * TransactionList, size_t Count) {
		size_t i = 0;
		while (i < Count) {
			if (!TransactionList[i].Exec()) {
				break;
			}
			++i;
		}
		if (i == Count) {
			return true;
		}
		while (i) {
			TransactionList[--i].Undo();
		}
		return false;
	}

	template <typename tTransactional>
	X_STATIC_INLINE bool BatchExec(tTransactional && TransactionalRef) {
		return std::forward<tTransactional>(TransactionalRef).Exec();
	}

	template <typename tTransactional, typename... tTransactionalRefs>
	X_STATIC_INLINE bool BatchExec(tTransactional && TransactionalRef, tTransactionalRefs &&... TransactionRefs) {
		if (!std::forward<tTransactional>(TransactionalRef).Exec()) {
			return false;
		}
		if (!BatchExecute(std::forward<tTransactionalRefs>(TransactionRefs)...)) {
			std::forward<tTransactional>(TransactionalRef).Undo();
			return false;
		}
		return true;
	}
};

X_END
