//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2015.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// Boost test of the expected library.


#include <string>
#include "boost/expected/expected.hpp"

using namespace std;
using namespace boost;

int main()
{
    expected<string, int> exp = make_unexpected(0);

    if (!exp)
        return exp.get_unexpected().value();

    return 0;
}
