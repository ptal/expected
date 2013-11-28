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
# include <initializer_list>

#include <boost/expected/expected.hpp>

enum State
{
    sDefaultConstructed,
    sValueCopyConstructed,
    sValueMoveConstructed,
    sCopyConstructed,
    sMoveConstructed,
    sMoveAssigned,
    sCopyAssigned,
    sValueCopyAssigned,
    sValueMoveAssigned,
    sMovedFrom,
    sValueConstructed
};

struct OracleVal
{
    State s;
    int i;
    OracleVal(int i = 0) : s(sValueConstructed), i(i) {}
};

struct Oracle
{
    State s;
    OracleVal val;

    Oracle() : s(sDefaultConstructed) {}
    Oracle(const OracleVal& v) : s(sValueCopyConstructed), val(v) {}
    Oracle(OracleVal&& v) : s(sValueMoveConstructed), val(std::move(v)) {v.s = sMovedFrom;}
    Oracle(const Oracle& o) : s(sCopyConstructed), val(o.val) {}
    Oracle(Oracle&& o) : s(sMoveConstructed), val(std::move(o.val)) {o.s = sMovedFrom;}

    Oracle& operator=(const OracleVal& v) { s = sValueCopyConstructed; val = v; return *this; }
    Oracle& operator=(OracleVal&& v) { s = sValueMoveConstructed; val = std::move(v); v.s = sMovedFrom; return *this; }
    Oracle& operator=(const Oracle& o) { s = sCopyConstructed; val = o.val; return *this; }
    Oracle& operator=(Oracle&& o) { s = sMoveConstructed; val = std::move(o.val); o.s = sMovedFrom; return *this; }
};

using namespace boost;

class test_exception : public std::exception
{
};

BOOST_AUTO_TEST_SUITE(except_default_constructor)

BOOST_AUTO_TEST_CASE(except_default_constructor)
{
  // From value constructor.
  expected<int> e;
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK(e.valid());
  BOOST_CHECK(! ! e);
  BOOST_CHECK(bool(e));
}

BOOST_AUTO_TEST_CASE(except_default_constructor_constexpr)
{
  // From value constructor.
  BOOST_CONSTEXPR expected<int,int> e;
  BOOST_CHECK(e.valid());
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(except_expected_constructors)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<int> e(5);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_cnv_value)
{
  OracleVal v;
  expected<Oracle> e(v);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK(! ! e) ;
  BOOST_CHECK(e.valid());
  BOOST_CHECK(bool(e));
  BOOST_CHECK_EQUAL(e.get().s,  sMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sValueConstructed);

  expected<Oracle> e2(std::move(v));
  BOOST_REQUIRE_NO_THROW(e2.get());
  BOOST_CHECK(! ! e2) ;
  BOOST_CHECK(e2.valid());
  BOOST_CHECK(bool(e2));
  BOOST_CHECK_EQUAL(e.get().s, sMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sMovedFrom);

}

BOOST_AUTO_TEST_CASE(expected_from_in_place_value)
{
  OracleVal v;
  expected<Oracle> e{in_place_t{}, v};
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK(! ! e) ;
  BOOST_CHECK(e.valid());
  BOOST_CHECK(bool(e));
  BOOST_CHECK_EQUAL(e.get().s, sValueCopyConstructed);
  BOOST_CHECK_EQUAL(v.s, sValueConstructed);

  expected<Oracle> e2{in_place_t{}, std::move(v)};
  BOOST_REQUIRE_NO_THROW(e2.get());
  BOOST_CHECK(! ! e2) ;
  BOOST_CHECK(e2.valid());
  BOOST_CHECK(bool(e2));
  BOOST_CHECK_EQUAL(e2.get().s, sValueMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sMovedFrom);

}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From exceptional constructor.
  expected<int> e(exceptional, test_exception());
  BOOST_REQUIRE_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_copy_value)
{
  // From copy constructor.
  expected<int> ef(5);
  expected<int> e(ef);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_copy_exception)
{
  // From exceptional constructor.
  expected<int> ef(exceptional, test_exception());
  expected<int> e(ef);
  BOOST_REQUIRE_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_in_place)
{
  // From in_place2 constructor.
  expected<std::string> e(in_place2, "in_place2");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
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

BOOST_AUTO_TEST_CASE(expected_from_moved_value)
{
  // From move value constructor.
  std::string value = "my value";
  expected<std::string> e = std::move(value);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "my value");
  BOOST_CHECK_EQUAL(*e, "my value");
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
    expected<int> e(exceptional);

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
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From exceptional constructor.
  expected<int, std::error_condition> e(exceptional, std::make_error_condition(std::errc::invalid_argument));
  auto error_from_except_check = [](const bad_expected_access<std::error_condition>& except)
  {
    return std::errc(except.error().value()) == std::errc::invalid_argument;
  };
  BOOST_REQUIRE_EXCEPTION(e.get(), bad_expected_access<std::error_condition>, error_from_except_check);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(except_valid)

BOOST_AUTO_TEST_CASE(except_valid_constexpr)
{
  // From value constructor.
  BOOST_CONSTEXPR expected<int,int> e;
  BOOST_CONSTEXPR bool b = e.valid();
  BOOST_CHECK(b);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(except_expected_assignment)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  expected<int> e(5);
  BOOST_CHECK_EQUAL(e.get(), 5);

  // From value assignment.
  e = 8;
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), 8);
  BOOST_CHECK_EQUAL(*e, 8);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_copy_expected)
{
  expected<int> e(5);
  expected<int> e2(8);

  // From value assignment.
  e = e2;
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), 8);
  BOOST_CHECK_EQUAL(*e, 8);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_moved_expected)
{
  expected<std::string> e("e");
  expected<std::string> e2("e2");

  // From value assignment.
  e = std::move(e2);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "e2");
  BOOST_CHECK_EQUAL(*e, "e2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));

  BOOST_REQUIRE_NO_THROW(e2.get());
  BOOST_CHECK_EQUAL(e2.get(), "");
  BOOST_CHECK_EQUAL(*e2, "");
  BOOST_CHECK(e2.valid());
  BOOST_CHECK(static_cast<bool>(e2));
}

BOOST_AUTO_TEST_CASE(expected_from_im_place)
{
  // From in_place2 constructor.
  expected<std::string> e(in_place2, "in_place2");
  BOOST_CHECK_EQUAL(e.get(), "in_place2");

  // From emplace method.
  e.emplace("emplace method");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "emplace method");
  BOOST_CHECK_EQUAL(*e, "emplace method");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_move_value)
{
  expected<std::string> e("v");

  std::string value = "my value";
  // From assignment operator.
  e = std::move(value);
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "my value");
  BOOST_CHECK_EQUAL(*e, "my value");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_factories)

BOOST_AUTO_TEST_CASE(expected_from_in_place)
{
  // From in_place2 factory.
  auto e = make_expected<std::string>("in_place2");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_in_place_error)
{
  // From in_place2 factory.
  auto e = make_expected<std::string, std::error_condition>("in_place2");
  BOOST_REQUIRE_NO_THROW(e.get());
  BOOST_CHECK_EQUAL(e.get(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
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
    expected<int> e = make_expected_from_error<int>();

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
    auto throw_lambda = [](){ make_expected_from_error<int,std::error_condition>();};

    BOOST_CHECK_THROW(throw_lambda(), test_exception);
  }
}

BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From exceptional constructor.
  auto e = make_expected_from_error<int>(std::make_error_condition(std::errc::invalid_argument));
  auto error_from_except_check = [](const bad_expected_access<std::error_condition>& except)
  {
    return std::errc(except.error().value()) == std::errc::invalid_argument;
  };
  BOOST_CHECK_EXCEPTION(e.get(), bad_expected_access<std::error_condition>, error_from_except_check);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From exceptional constructor.
  auto e = make_expected_from_error<int>(test_exception());
  BOOST_CHECK_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  auto e = make_expected_from_error<int>(boost::copy_exception(test_exception()));
  BOOST_CHECK_THROW(e.get(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(error_expected_modifier)

BOOST_AUTO_TEST_CASE(expected_swap_value)
{
  // From value constructor.
  expected<int> e(5);
  expected<int> e2(8);

  e.swap(e2);

  BOOST_CHECK_EQUAL(e.get(), 8);
  BOOST_CHECK_EQUAL(e2.get(), 5);

  e2.swap(e);

  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK_EQUAL(e2.get(), 8);
}

BOOST_AUTO_TEST_CASE(expected_swap_exception)
{
  // From value constructor.
  expected<int> e = make_expected_from_error<int>(std::invalid_argument("e"));
  expected<int> e2 = make_expected_from_error<int>(std::invalid_argument("e2"));

  e.swap(e2);

  auto equal_to_e = [](const std::invalid_argument& except) { return std::string(except.what()) == "e"; };
  auto equal_to_e2 = [](const std::invalid_argument& except) { return std::string(except.what()) == "e2"; };

  BOOST_CHECK_EXCEPTION(e.get(), std::invalid_argument, equal_to_e2);
  BOOST_CHECK_EXCEPTION(e2.get(), std::invalid_argument, equal_to_e);

  e2.swap(e);

  BOOST_CHECK_EXCEPTION(e.get(), std::invalid_argument, equal_to_e);
  BOOST_CHECK_EXCEPTION(e2.get(), std::invalid_argument, equal_to_e2);
}

BOOST_AUTO_TEST_CASE(expected_swap_function_value)
{
  // From value constructor.
  expected<int> e(5);
  expected<int> e2(8);

  swap(e, e2);

  BOOST_CHECK_EQUAL(e.get(), 8);
  BOOST_CHECK_EQUAL(e2.get(), 5);

  swap(e, e2);

  BOOST_CHECK_EQUAL(e.get(), 5);
  BOOST_CHECK_EQUAL(e2.get(), 8);
}

BOOST_AUTO_TEST_SUITE_END()


