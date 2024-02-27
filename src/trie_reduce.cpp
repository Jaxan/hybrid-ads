/*
 * Reads in a test suite (maybe several in a row), and reduces it by adding
 * all sequences to a trie, removing redundancies. It then outputs the reduced
 * test suite, and its size to stderr.
 *
 * usage, e.g.:
 *
 *   cat test-suite-1.txt test-suite-2.txt | ./trie_reduce
 *
 */

#include <trie.hpp>
#include <types.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char * argv[]) {
	trie<string> test_suite;

	unsigned int symbol_count = 0;
	unsigned int word_count = 0;
	string line;
	string symbol;
	vector<string> symbols;
	while (getline(cin, line)) {
		symbols.clear();
		stringstream ss(line);
		while (ss >> symbol) {
			symbols.push_back(symbol);
			symbol_count++;
		}

		test_suite.insert(symbols);
		word_count++;
	}

	clog << "words in:    " << setw(9) << word_count << endl;
	clog << "symbols in:  " << setw(9) << symbol_count << endl;

	symbol_count = 0;
	word_count = 0;
	const auto output_word = [&symbol_count, &word_count](const auto & w) {
		for (const auto & x : w) {
			cout << x << ' ';
			symbol_count++;
		}
		cout << endl;
		word_count++;
	};

	test_suite.for_each(output_word);

	clog << "words out:   " << setw(9) << word_count << endl;
	clog << "symbols out: " << setw(9) << symbol_count << endl;
}
