#include "./io_reactor.hpp"

X_BEGIN

namespace __network_detail__ {

	static_assert(std::is_standard_layout_v<xIoContextEventNode>);
	static_assert(std::is_standard_layout_v<__xIoReactor__>);

}  // namespace __network_detail__

#if defined(X_SYSTEM_WINDOWS)
X_API void Retain(xOverlappedIoBuffer * IBP) {
	++IBP->ReferenceCount;
}
X_API xOverlappedIoBuffer * Release(xOverlappedIoBuffer * IBP) {
	if (!--IBP->ReferenceCount) {
		if (IBP->Cleaner) {
			(*IBP->Cleaner)(IBP);
		}
		delete IBP;
		return nullptr;
	}
	return IBP;
};
#endif

X_API_MEMBER bool xSocketIoReactor::Init() {
#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)
	return true;
#elif defined(X_SYSTEM_WINDOWS)
	IBP = new (std::nothrow) std::decay_t<decltype(*IBP)>();
	if (!IBP) {
		return false;
	}
	IBP->Reactor              = this;
	IBP->Reader.Native.Outter = IBP;
	IBP->Writer.Native.Outter = IBP;
	return true;
#endif
}

X_API_MEMBER void xSocketIoReactor::Clean() {
	auto & WriteBufferChain = IBP->WriteBufferChain;
	while (auto BP = WriteBufferChain.Pop()) {
		delete BP;
	}
#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)
	return;
#elif defined(X_SYSTEM_WINDOWS)
	Release(Steal(IBP));
#endif
}

X_END
