#pragma once
#include "./core_min.hpp"

#include <functional>

X_BEGIN

template <auto... V>
class xNoop;
template <>
class xNoop<> {
private:
	template <typename... Args>
	using F = void (*)(Args...);

public:
	template <typename... Args>
	operator F<Args...>() const {
		return [](Args...) {};
	}
};
template <auto V>
class xNoop<V> {
protected:
	template <typename... Args>
	using F = decltype(V) (*)(Args...);
	template <typename... Args>
	using FF = std::function<decltype(V)(Args...)>;

public:
	template <typename... Args>
	operator F<Args...>() const {
		return [](Args...) { return V; };
	}
	template <typename... Args>
	operator FF<Args...>() const {
		return typename xNoop<V>::F<Args...>(*this);
	}
};
template <auto... V>
constexpr const auto Noop = xNoop<V...>();

X_END
