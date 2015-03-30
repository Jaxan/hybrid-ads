Yannakakis
==========

An algorithm to construct an adaptive distinguishing sequence for a mealy
machine. If it does not exist, a partial sequence will be generated, which is
still useful for generating a seperating set (in the sense of Lee and
Yannakakis). The partial leaves will be augmented via the classical
seperating sequences.


## Building

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```

Then every .cpp file in the src directory will be built and generate an
executable in the build directory. Note that you'll need c++14, but clang in
Mac OSX will understand that (and if not, you'll have to update Xcode).


## Java

For now the java code, which acts as a bridge between LearnLib and this c++ 
tool, is included here. But it should earn its own repo at some point. Also, my 
javanese is a bit rusty...
