#include <partition.hpp>

#include <iostream>

using namespace std;

int main(int argc, char *argv[]){
	partition_refine p(50);

	auto bs = p.refine(p.find(0), [](size_t x){ return x % 1; }, 1);
	cout << elems_in(bs) << endl;

	bs = p.refine(p.find(0), [](size_t x){ return x % 2; }, 2);
	cout << elems_in(bs) << endl;

	bs = p.refine(p.find(0), [](size_t x){ return x % 3; }, 3);
	cout << elems_in(bs) << endl;

	bs = p.refine(p.find(0), [](size_t x){ return x % 5; }, 5);
	cout << elems_in(bs) << endl;

	bs = p.refine(p.find(0), [](size_t x){ return x % 7; }, 7);
	cout << elems_in(bs) << endl;

	for(int i = 0; i < 50; ++i){
		cout << i << ":\t";
		const auto block = *p.find(i);
		for(auto && x : block) cout << x << " ";
		cout << endl;
	}
}
