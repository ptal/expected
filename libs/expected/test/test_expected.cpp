//! \file test_expected.cpp

// Copyright Pierre Talbot 2013.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// Boost test of the expected library.

// See http://www.boost.org/doc/libs/1_46_1/libs/test/doc/html/utf/testing-tools/reference.html

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

#include "../../../boost/expected/expected.hpp"

using namespace boost;

BOOST_AUTO_TEST_SUITE(expected_base)

BOOST_AUTO_TEST_CASE(expected_constructor)
{
  expected<int> e = 5;
}

BOOST_AUTO_TEST_SUITE_END()
