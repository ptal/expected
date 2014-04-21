//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/expected/expected.hpp>
#include <boost/expected/algorithms/value.hpp>
#include <utility>

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm value"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE



#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace boost::expected_alg;

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(Value)
////////////////////////////////////////////////////////////////////////////////////////////////////
// T const& value(expected<T> &&)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Value_Valued_RValue)
{
  expected<std::exception_ptr, int> ei = 1;
  auto ej = ei.catch_error(thrower<int>());
  BOOST_CHECK (*ej == 1);
  int i = value(std::move(ei));
  BOOST_CHECK_EQUAL (i, 1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// T const&value(expected<T> const&)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Value_Valued_LValueConst)
{
  const expected<std::exception_ptr, int> ei = 1;
  int i = value(ei);
  BOOST_CHECK_EQUAL (i, 1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Value_Valued_LValueAssign)
{
  expected<std::exception_ptr, int> ei = 1;
  ei.value() = 2;
  //value(ei) = 2; // fails
  BOOST_CHECK_EQUAL (*ei, 2);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// T& value(expected<T> &)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Value_Valued_LvalueNonConst)
{
  expected<std::exception_ptr, int> ei = 1;
  int i = value(ei);
  BOOST_CHECK_EQUAL (i, 1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Value_Unexpected_LValue)
{
  expected<std::exception_ptr, int> ei;
  BOOST_REQUIRE_THROW(value(ei), expected_default_constructed);
}
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
