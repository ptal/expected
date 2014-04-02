//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm value_or_call"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected.hpp>
#include <boost/expected/algorithms/value_or_call.hpp>
#include <utility>

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace boost::expected_alg;

int f0() {Êreturn 0; }

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(ValueOrCall)
////////////////////////////////////////////////////////////////////////////////////////////////////
// value_or_call(expected<T>, F)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOrCall_Valued)
{
  expected<int> ei = 1;
  int i = value_or_call(std::move(ei), f0);
  BOOST_CHECK (i == 1);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOrCall_Unexpected)
{
  expected<int> ei;
  int i = value_or_call(std::move(ei), f0);
  BOOST_CHECK (i == 0);
}
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
