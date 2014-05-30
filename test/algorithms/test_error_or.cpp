//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm error_or"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected.hpp>
#include <boost/expected/algorithms/error_or.hpp>
#include <utility>

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace boost::expected_alg;

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(ErrorOr)
////////////////////////////////////////////////////////////////////////////////////////////////////
// error_or(expected<T>, T)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ErrorOr_Valued)
{
  expected<std::string, int> ei = 1;
  std::string err = error_or(std::move(ei),"0");
  BOOST_CHECK (err == "0");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ErrorOr_Valued2)
{
  expected<std::string, int> ei = 1;
  std::string err = error_or(ei,"0");
  BOOST_CHECK (err == "0");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ErrorOr_Unexpected)
{
  expected<std::string, int> ei;
  std::string err = error_or(std::move(ei), "0");
  BOOST_CHECK (err == "");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ErrorOr_Unexpected2)
{
  expected<std::string, int> ei;
  std::string err = error_or(ei, "0");
  BOOST_CHECK (err == "");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ErrorOr_Unexpected3)
{
  expected<std::string, int> ei;
  std::string err = error_or(std::move(ei), "0");
  BOOST_CHECK (err != "0");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
