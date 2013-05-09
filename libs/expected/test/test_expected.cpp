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

#include <stdexcept>
#include <exception>
#include <system_error>

#include "../../../boost/expected/expected.hpp"

using namespace boost;

class test_exception : public std::exception
{
};

BOOST_AUTO_TEST_SUITE(except_expected_constructors)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<int> e(5);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From exceptional constructor.
  expected<int> e(exceptional, test_exception());
  BOOST_REQUIRE_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_emplace)
{
  // From emplace constructor.
  expected<std::string> e(emplace, "emplace");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "emplace");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  expected<int> e(exceptional, boost::copy_exception(test_exception()));
  BOOST_REQUIRE_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_move_value)
{
  // From move value constructor.
  std::string value = "my value";
  expected<std::string> e = std::move(value);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "my value");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_catch_block)
{
  // From catch block
  try
  {
    throw test_exception();
  }
  catch(...)
  {
    expected<int> e;

    BOOST_REQUIRE_THROW(e.get(), std::exception);
    BOOST_CHECK_EQUAL(e.valid(), false);
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(error_expected_constructors)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<int, std::error_condition> e(5);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From exceptional constructor.
  expected<int, std::error_condition> e(exceptional, std::make_error_condition(std::errc::invalid_argument));
  auto error_from_except_check = [](const bad_expected_access<std::error_condition>& except)
  { 
    return std::errc(except.error().value()) == std::errc::invalid_argument ;
  };
  BOOST_REQUIRE_EXCEPTION(e.get(), bad_expected_access<std::error_condition>, error_from_except_check);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_factories)

BOOST_AUTO_TEST_CASE(expected_from_emplace)
{
  // From emplace factory.
  auto e = make_expected<std::string>("emplace");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "emplace");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_emplace_error)
{
  // From emplace factory.
  auto e = make_expected<std::string, std::error_condition>("emplace");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "emplace");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_exception_catch)
{
  // From catch block
  try
  {
    throw test_exception();
  }
  catch(...)
  {
    expected<int> e = make_exceptional_expected<int>();

    BOOST_REQUIRE_THROW(e.get(), std::exception);
    BOOST_CHECK_EQUAL(e.valid(), false);
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
  }
}

BOOST_AUTO_TEST_CASE(expected_from_error_catch_exception)
{ 
  // From catch block
  try
  {
    throw test_exception();
  }
  catch(...)
  {
    //auto throw_lambda = [](){make_exceptional_expected<int,std::error_condition>();};
    //BOOST_REQUIRE_THROW(throw_lambda(), boost::exception);
  }
}

BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From exceptional constructor.
  auto e = make_exceptional_expected<int>(std::make_error_condition(std::errc::invalid_argument));
  auto error_from_except_check = [](const bad_expected_access<std::error_condition>& except)
  { 
    return std::errc(except.error().value()) == std::errc::invalid_argument ;
  };
  BOOST_REQUIRE_EXCEPTION(e.get(), bad_expected_access<std::error_condition>, error_from_except_check);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From exceptional constructor.
  auto e = make_exceptional_expected<int>(test_exception());
  BOOST_REQUIRE_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  auto e = make_exceptional_expected<int>(boost::copy_exception(test_exception()));
  BOOST_REQUIRE_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(error_expected_modifier)

BOOST_AUTO_TEST_CASE(expected_swap_value)
{
  // From value constructor.
  expected<int, std::error_condition> e(5);
  expected<int, std::error_condition> e2(8);

  e.swap(e2);

  BOOST_CHECK_EQUAL(e.get(), 8);
  BOOST_CHECK_EQUAL(e2.get(), 5);

  e2.swap(e);

  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK_EQUAL(e2.get(), 8);
}

BOOST_AUTO_TEST_SUITE_END()


