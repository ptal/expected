//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/expected/algorithms/if_then_else.hpp>

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm  if_then_else"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
using namespace boost::expected_alg;

std::string to_string2(int ) { return std::string("1");}
std::string to_error(std::exception_ptr ) { return std::string("2");}

expected<int, std::string> to_expected_string_ok(int ) { return expected<int, std::string>(in_place_t{}, "1");}
expected<int, std::string> to_expected_error_ok(int ) { return expected<int, std::string>(in_place_t{}, "2");}
expected<int, std::string> to_expected_string_ko(int ) { return expected<int, std::string>{unexpect, 1};}
expected<int, std::string> to_expected_error_ko(int ) { return expected<int, std::string>{unexpect, 2};}

///////////////////////////
BOOST_AUTO_TEST_SUITE(Copyable)
  BOOST_AUTO_TEST_SUITE(Copyable_NotNested)
    BOOST_AUTO_TEST_CASE(Copyable_NotNested_Valued)
    {
      auto e = if_then_else(make_expected(1), to_string2, to_error);
      BOOST_CHECK (e);
      BOOST_CHECK (*e == "1");
    }
    BOOST_AUTO_TEST_CASE(Copyable_NotNested_Unexpected)
    {
      auto e = if_then_else(expected<std::exception_ptr, int>{make_unexpected(1)}, to_string2, to_error);
      BOOST_CHECK (e);
      BOOST_CHECK (*e == "2");
    }
  BOOST_AUTO_TEST_SUITE_END()

#ifndef BOOST_EXPECTED_NO_IF_THEN_ELSE
  BOOST_AUTO_TEST_SUITE(Copyable_Nested)
    BOOST_AUTO_TEST_SUITE(Copyable_Nested_OkOk)
      BOOST_AUTO_TEST_CASE(Copyable_Nested_OkOk_Valued)
      {
        auto e = if_then_else(expected<int,int>(1), to_expected_string_ok, to_expected_error_ok);
        BOOST_CHECK (e);
        BOOST_CHECK (*e == std::string("1"));
      }
      BOOST_AUTO_TEST_CASE(Copyable_Nested_OkOk_Unexpected)
      {
        auto e = if_then_else(expected<int,int>{unexpect, 1}, to_expected_string_ok, to_expected_error_ok);
        BOOST_CHECK (e);
        BOOST_CHECK (*e == std::string("2"));
      }
    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE(Copyable_Nested_OkKo)
      BOOST_AUTO_TEST_CASE(Copyable_Nested_OkKo_Valued)
      {
        auto e = if_then_else(expected<int,int>(1), to_expected_string_ok, to_expected_error_ko);
        BOOST_CHECK (e);
        BOOST_CHECK (*e == std::string("1"));
      }
      BOOST_AUTO_TEST_CASE(Copyable_Nested_OkKo_Unexpected)
      {
        expected<int,int> e0{unexpect, 1};
        expected<int, expected<int, std::string>> e1 = e0.fmap(to_expected_string_ok);
        expected<int, expected<int, std::string>> e2 = e1.recover(to_expected_error_ko);
        expected<int, std::string> e3 = unwrap(e2);
        BOOST_CHECK (!e3 && (e3.error() == 2));
        expected<int, std::string> e = if_then_else(expected<int,int>{unexpect, 1}, to_expected_string_ok, to_expected_error_ko);
        BOOST_CHECK (!e);
        BOOST_CHECK (e.error() == 2);
      }
    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE(Copyable_Nested_KoOk)
      BOOST_AUTO_TEST_CASE(Copyable_Nested_KoOk_Valued)
      {
        auto e = if_then_else(expected<int,int>(1), to_expected_string_ko, to_expected_error_ok);
        BOOST_CHECK (!e);
        BOOST_CHECK (e.error() == 1);
      }
      BOOST_AUTO_TEST_CASE(Copyable_Nested_KoOk_Unexpected)
      {
        auto e = if_then_else(expected<int,int>{unexpect, 1}, to_expected_string_ko, to_expected_error_ok);
        BOOST_CHECK (e);
        BOOST_CHECK (*e == std::string("2"));
      }
    BOOST_AUTO_TEST_SUITE_END()


    BOOST_AUTO_TEST_SUITE(Copyable_Nested_KoKo)
      BOOST_AUTO_TEST_CASE(Copyable_Nested_KoKo_Valued)
      {
        auto e = if_then_else(expected<int,int>(1), to_expected_string_ko, to_expected_error_ko);
        BOOST_CHECK (!e);
        BOOST_CHECK (e.error() == 1);
      }
      BOOST_AUTO_TEST_CASE(Copyable_Nested_KoKo_Unexpected)
      {
        auto e = if_then_else(expected<int,int>{unexpect, 1}, to_expected_string_ko, to_expected_error_ko);
        BOOST_CHECK (!e);
        BOOST_CHECK (e.error() == 2);
      }
    BOOST_AUTO_TEST_SUITE_END()

  BOOST_AUTO_TEST_SUITE_END()
#endif

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(MovableOnly)
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(Void)
BOOST_AUTO_TEST_SUITE_END()