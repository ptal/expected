//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm has_error"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected.hpp>
#include <boost/expected/algorithms/has_error.hpp>
#include <utility>

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace boost::expected_alg;

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(HasError)
////////////////////////////////////////////////////////////////////////////////////////////////////
// error_or(expected<T>, T)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(HasError_Valued)
{
  expected<int, std::string> ei = 1;
  BOOST_CHECK (( ! has_error(ei, "") ));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(HasError_Unexpected)
{
  expected<int, std::string> ei;
  BOOST_CHECK (( has_error(ei, "") ));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(HasError_Unexpected2)
{
  expected<int, std::string> ei;
  BOOST_CHECK (( ! has_error(ei, "a") ));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
