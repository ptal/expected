!!! WARINING !!!

This repository is abandoned.

This proposal has moved to this repository https://github.com/viboes/std-make/blob/master/doc/proposal/expected/ and https://github.com/viboes/std-make/tree/master/doc/proposal/expected.

Please, add any issues in the new repository.

!!! WARINING !!!

Expected
========


Expected has been proposed to the C++ standard committee. The current document is N4109 (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4109.pdf). 

A library for expected objects for C++17. This is the reference implementation of proposal N4109 
(see see https://github.com/ptal/std-expected-proposal). 

Supported compilers
-------------------
* Clang 3.2
* G++ 4.8.0 (and probably later)
* VS14 CTP 3. Note that this compiler cannot do enough constexpr for Expected (or Boost for that matter) to turn it on, but it appears that MSVC doesn't mind and all unit tests pass anyway. It is possible VS14 RTM will do enough constexpr that it can be turned on.
* VS2013 which has the same limitations as VS14 (no constexpr). This uses an unrestricted union emulation which works well enough.

Usage
-----
For usage examples and the overview see https://github.com/ptal/std-expected-proposal

Differences from N4109
----------------------
* The constructor taking initializer_list argument is not constexpr. This is because initializer_list operations are not constexpr in C++11.
* Member function value_or does not have rvalue reference overload in GCC inferior to 4.8.1. This is because rvalue overloading on *this is not supported until GCC 4.8.1.
