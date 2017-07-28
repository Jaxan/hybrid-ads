Hybrid adaptive distinguishing sequences
========================================

In FSM-based test generation, a key feature of smart tests are efficient state
identifiers. This tool generates a test suite based on the adaptive
distinguishing sequences as described by Lee and Yannakakis (1994). Many Mealy
machines do not admit such sequences, but luckily the sequences can be extended
in order to obtain a complete test suite.

*NOTE*: This repository was originally located at
[here](https://gitlab.science.ru.nl/moerman/Yannakakis).
But I realised that installing the software was not as easy as it should be,
and so I cleaned up dependencies, while keeping the original repository fixed.
(Also some people might still use the old link.)


## Introduction

This tool will generate a complete test suite for a given specification. The
specification should be given as a completely-specified, deterministic Mealy
machine (or finite state machine, FSM). Also the implementation is assumed to
be complete and deterministic. If the implementation passes the test suite,
then it is guaranteed to be equivalent or to have at least k more states.
The parameter k can be chosen. The size of the test suite is polynomial in the
number of states of the specification, but exponential in k.

There are many ways to generate such a complete test suite. Many variations,
W, Wp, HSI, ADS, UIOv, ..., exist in literature. Implemented here (as of
writing) are HSI, ADS and the hybrid-ADS method. Since the ADS method is not
valid for all Mealy machines, this tool extends the method to be complete,
hence the name "hybrid-ADS". This is a new method (although very similar to the
HSI and ADS methods).

In addition to choosing the state identifier, one can also choose how the
prefixes for the tests are generated. Typically, one will use shortest paths,
but longer ones can be used too.

All algorithms implemented here can be randomised, which can greatly reduce
the size of the test suite. 

Most of the algorithms are found in the directory `lib/` and their usage is best
illustrated in `src/main.cpp`. The latter can be used as a stand-alone tool.
The input to the executable are `.dot` files (of a specific type). Please look
at the provided example to get started.


## Building

There are no dependencies to install.
You can build the tool with `cmake`:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```

I hope most of the code is portable c++11. But I may have used some c++14
features. (If this is a problem for you, please let me know.)


## Java

For now the java code, which acts as a bridge between LearnLib and this c++
tool, is included here (can be out-dated). But it should earn its own repo at
some point. Also, my javanese is a bit rusty...


## Implementation details

Currently states and inputs are encoded internally as integer values (because
this enables fast indexing). Only for I/O, maps are used to translate between
integers and strings. To reduce the memory footprint `uint16_t`s are used, as
the range is big enough for our use cases (`uint8_t` is clearly too small for
the number of states, but could be used for alphabets).

A prefix tree (or trie) is used to reduce the test suite, by removing common
prefixes. However, this can quickly grow in size. Be warned!


## TODO

* Implement a proper radix tree (or Patricia tree) to reduce memory usage.
* Implement the SPY method for finding smarter prefixes.
* Compute independent structures in parallel (this was done in the first
  version of the tool).
* Implement the O(n log n) algorithm to find state identifiers, instead of the
  current (roughly) O(n^2) algorithm.


## License

See `LICENSE`
