#include <phantom.hpp>
#include <iostream>

template <typename T>
using phantom_int = phantom<int, T>;

struct state_tag;
using state = phantom_int<state_tag>;

struct input_tag;
using input = phantom_int<input_tag>;

int main(int argc, char *argv[]){
	state x = 5;
	x += 5;

	input y = 9;
	y -= 9;
	y++;

	std::cout << (x - 1) << std::endl;
	std::cout << (y << 1) << std::endl;
	// std::cout << x+y << std::endl; // does not compile

	for(input i = 0; i < 10; ++i){
		std::cout << i << ", ";
	}
	std::cout << std::endl;
}

