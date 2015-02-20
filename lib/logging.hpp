#pragma once

#include <mutex>

// Works particularly nice with lambda's, as they give naturally unique types :)
template <typename F>
void fire_once(F && f){
	static std::once_flag flag;
	std::call_once(flag, f);
}
