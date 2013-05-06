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

#include "../../../boost/expected/expected.hpp"

using namespace boost;

class test_exception : public std::exception
{
};

BOOST_AUTO_TEST_SUITE(expected_constructor)

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
    //BOOST_REQUIRE_THROW(e.get(), test_exception);
    BOOST_CHECK_EQUAL(e.valid(), false);
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
  }
}

BOOST_AUTO_TEST_SUITE_END()
