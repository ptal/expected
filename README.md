Expected
========
A library for expected objects for C++14. This is the reference implementation of proposal NXXXX 
(see see https://github.com/ptal/std-expected-proposal). 
Expected has not been proposed yet to the C++ standard commitee. 

Supported compilers
-------------------
Clang 3.1, G++ 4.7.1 (and probably later)

Usage
-----
For usage examples and the overview see https://github.com/ptal/std-expected-proposal

Differences from NXXXX
----------------------
* The constructor taking initializer_list argument is not constexpr. This is because initializer_list operations are not constexpr in C++11.
* Member function value_or does not have rvalue reference overload in GCC inferior to 4.8.1. This is because rvalue overloding on *this is not supported until GCC 4.8.1.
