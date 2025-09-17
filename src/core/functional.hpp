#pragma once
#include "./core_min.hpp"

#include <functional>

X_BEGIN

namespace __detail__ {

	template <auto... V>
	class xNoop;

	template <>
	class xNoop<> {
	private:
		template <typename... Args>
		using FF = std::function<void(Args...)>;

	public:
		template <typename... Args>
		operator FF<Args...>() const {
			return [](Args...) {};
		}
	};

	template <auto V>
	class xNoop<V> {
	protected:
		template <typename... Args>
		using FF = std::function<decltype(V)(Args...)>;

	public:
		template <typename... Args>
		operator FF<Args...>() const {
			return [](Args...) { return V; };
		}
	};

}  // namespace __detail__

template <auto... V>
constexpr const auto Noop = __detail__::xNoop<V...>();

template <typename R, typename Class, typename... tArgs>
auto Delegate(R (Class::*F)(tArgs...) const, const Class * P) {
	return [F, P](tArgs... args) { return (P->*F)(args...); };
}

template <typename R, typename Class, typename... tArgs>
auto Delegate(R (Class::*F)(tArgs...), Class * P) {
	return [F, P](tArgs... args) { return (P->*F)(args...); };
}

X_END
