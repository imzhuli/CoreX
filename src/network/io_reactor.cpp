#include "./io_reactor.hpp"

X_BEGIN

namespace __network_detail__ {

	static_assert(std::is_standard_layout_v<xIoContextEventNode>);
	static_assert(std::is_standard_layout_v<__xIoReactor__>);

}  // namespace __network_detail__

X_API_MEMBER bool xSocketIoReactor::Init() {
#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)
	return true;
#elif defined(X_SYSTEM_WINDOWS)
	IBP = new (std::nothrow) std::decay_t<decltype(*IBP)>;
	return IBP;
#endif
}

X_API_MEMBER void xSocketIoReactor::Clean() {
#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)
	return;
#elif defined(X_SYSTEM_WINDOWS)
	if (!--IBP->ReferenceCount) {
		delete Steal(IBP);
	}
#endif
}

X_END
