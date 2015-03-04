#pragma once

#include <boost/operators.hpp>

#include <iosfwd>

template <typename Base, typename T>
struct phantom : boost::operators<phantom<Base, T>>, boost::shiftable<phantom<Base, T>> {
	phantom() = default;
	phantom(Base y) : x(y) {}
	phantom(phantom const &) = default;
	phantom& operator=(phantom const &) = default;
	phantom(phantom &&) = default;
	phantom& operator=(phantom &&) = default;

	explicit operator Base() const { return x; }
	Base base() const { return x; }

	Base x;

#define IMPL(op)                         \
	phantom & operator op (phantom rh) { \
		x op rh.x;                       \
		return *this;                    \
	}

	IMPL(+=)
	IMPL(-=)
	IMPL(*=)
	IMPL(/=)
	IMPL(%=)
	IMPL(|=)
	IMPL(&=)
	IMPL(^=)
	IMPL(<<=)
	IMPL(>>=)
#undef IMPL

	phantom & operator++() { ++x; return *this; }
	phantom & operator--() { --x; return *this; }

	bool operator<(phantom rh) const { return x < rh.x; }
	bool operator==(phantom rh) const { return x == rh.x; }
};

template <typename B, typename T>
std::ostream & operator<<(std::ostream & out, phantom<B, T> p){ return out << p.x; }

template <typename B, typename T>
std::istream & operator>>(std::istream & in, phantom<B, T> & p){ return in >> p.x; }
