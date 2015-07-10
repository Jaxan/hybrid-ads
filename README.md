Yannakakis
==========

An algorithm to construct an adaptive distinguishing sequence for a mealy
machine. If it does not exist, a partial sequence will be generated, which is
still useful for generating a seperating set (in the sense of Lee and
Yannakakis). The partial leaves will be augmented via ordinary seperating
sequences. In effect, the resulting test method is an instantiation of the HSI-
method, which tends towards the DS-method.

Most of the algorithms are found in the directory `lib/` and their usage is best
illustrated in `src/main.cpp` or `src/methods.cpp`.

Currently states and inputs are encoded internally as integer values (because
this enables fast indexing). Only for I/O, maps are used to translate between
integers and strings. To reduce the memory footprint `uint16_t`s are used, as
the range is big enough for our use cases (`uint8_t` is clearly too small for
the number of states, but could be used for alphabets).


## Building

The only dependency is boost. In order to build the project, one can use cmake.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```

Then every .cpp file in the src directory will be built and generate an
executable in the build directory. Note that you'll need c++14, but clang in Mac
OSX will understand that (and if not, you'll have to update Xcode). The main
sourcefile (`src/main.cpp`) can also be built with c++11 (this is tested on some
commits on both Windows and linux).


## Java

For now the java code, which acts as a bridge between LearnLib and this c++ 
tool, is included here. But it should earn its own repo at some point. Also, my 
javanese is a bit rusty...


## License

See `LICENSE`
