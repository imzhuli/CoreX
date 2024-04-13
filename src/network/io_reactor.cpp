#include "./io_reactor.hpp"

X_BEGIN

namespace __network_detail__ {

	static_assert(std::is_standard_layout_v<xIoContextEventNode>);
	static_assert(std::is_standard_layout_v<__xIoReactor__>);

}  // namespace __network_detail__

X_END
