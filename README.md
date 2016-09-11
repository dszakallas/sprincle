# sprincle
Incremental graph query engine based on actors

Sprincle is a collection of CAF actors that can be used to build a Rete network for graph pattern matching.

## Prerequisites
Sprincle is depending on the following programs and libraries:

 - CMake
 - Boost
 - CAF

## Building
Sprincle is a header only library, but to make sure it works on your machine you should at least try to build and
run the tests.

On UNIX-like systems
```bash
git clone https://github.com/szdavid92/sprincle.git
cd sprincle
mkdir build && cd build
cmake .. # Generates Makefiles
make # Builds the tests
make test # Runs the tests
```

Usual errors include not finding the required libraries.

##Installing

After generating the Makefiles run
```
make install
```

Run with `sudo` if elevated privilages are needed to write to a system location.

