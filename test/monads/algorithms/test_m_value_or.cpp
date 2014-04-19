//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm value_or"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected_monad.hpp>
#include <boost/functional/monads/algorithms/value_or.hpp>
#include <utility>

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace boost::functional;

int value_or(int* i, int j) { return (i) ? *i : j; }

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(ValueOr)
////////////////////////////////////////////////////////////////////////////////////////////////////
// value_or(expected<T>, T)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOr_Valued)
{
  using namespace boost::functional::valued;

  expected<std::exception_ptr, int> ei = 1;
  int i = value_or(std::move(ei), 0);
  BOOST_CHECK (i == 1);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOr_Unexpected)
{
  using namespace boost::functional::valued;

  expected<std::exception_ptr, int> ei;
  int i = value_or(std::move(ei), 0);
  BOOST_CHECK (i == 0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOr_Valued2)
{
  using namespace boost::functional::valued;

  int v = 1;
  int* ptr = &v;
  int i = value_or(ptr, 0);
  BOOST_CHECK (i == 1);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOr_Unexpected2)
{
  using namespace boost::functional::valued;

  int* ptr = 0;
  int i = value_or(ptr, 0);
  BOOST_CHECK (i == 0);
}
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
