//! \file test_expected.cpp

// Copyright Pierre Talbot 2013.
// Copyright Vicente J. Botet Escriba 2013,2014.

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
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

#include <stdexcept>
#include <exception>
#include <system_error>
#include <initializer_list>
#include <string>
#include <iostream>
#include <system_error>

#include <boost/expected/expected_monad.hpp>
#include <boost/expected/error_exception.hpp>
#include <boost/expected/ensured_read.hpp>
#include <boost/functional/monads/adaptor.hpp>

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

struct Guard
{
    std::string val;
    Guard() : val{} {}
    explicit Guard(std::string s, int = 0) : val(s) {}
    Guard(const Guard&) = delete;
    Guard(Guard&&) = delete;
    void operator=(const Guard&) = delete;
    void operator=(Guard&&) = delete;
};

struct ExplicitStr
{
    std::string s;
    explicit ExplicitStr(const char* chp) : s(chp) {};
};

struct Date
{
    int i;
    Date() = delete;
    Date(int i) : i{i} {};
    Date(Date&& d) : i(d.i) { d.i = 0; }
    Date(const Date&) = delete;
    Date& operator=(const Date&) = delete;
    Date& operator=(Date&& d) { i = d.i; d.i = 0; return *this;};
};

template <class T>
struct MoveAware
{
  T val;
  bool moved;
  MoveAware(T val) : val(val), moved(false) {}
  MoveAware(MoveAware const&) = delete;
  MoveAware(MoveAware&& rhs) : val(rhs.val), moved(rhs.moved) {
    rhs.moved = true;
  }
  MoveAware& operator=(MoveAware const&) = delete;
  MoveAware& operator=(MoveAware&& rhs) {
    val = (rhs.val);
    moved = (rhs.moved);
    rhs.moved = true;
    return *this;
  }
};

using namespace boost;
using namespace boost::functional;

class test_exception : public std::exception
{
};

int throwing_fun(){ throw test_exception(); }
int nothrowing_fun(){ return 4; }

void void_throwing_fun(){ throw test_exception(); }
void do_nothing_fun(){}


BOOST_AUTO_TEST_SUITE(except_default_constructor)

BOOST_AUTO_TEST_CASE(except_default_constructor2)
{
  // From value constructor.
  expected<std::exception_ptr, int> e {};
  try {
    e.value();
  } catch (expected_default_constructed& ex )  {
  } catch(...) {
    BOOST_CHECK(false);
  };
  BOOST_CHECK(!e.valid());
  BOOST_CHECK(! e);
  BOOST_CHECK(! static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(except_default_constructor)
{
  // From value constructor.
  expected<std::exception_ptr, int> e;
  try {
    e.value();
  } catch (expected_default_constructed& ex )  {
  } catch(...) {
    BOOST_CHECK(false);
  };
  BOOST_CHECK(!e.valid());
  BOOST_CHECK(! e);
  BOOST_CHECK(! static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(except_default_constructor_constexpr)
{
  // From value constructor.
  BOOST_CONSTEXPR expected<int,int> e;
  BOOST_CHECK(!e.valid());
}


BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(except_expected_constructors)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<std::exception_ptr, int> e(5);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_value2)
{
  // From value constructor.
  expected<std::exception_ptr, int> e(5);
  e = {};
  try {
    e.value();
  } catch (expected_default_constructed& ex )  {
  } catch(...) {
    BOOST_CHECK(false);
  };
  BOOST_CHECK(! e.valid());
}
BOOST_AUTO_TEST_CASE(expected_from_cnv_value)
{
  OracleVal v;
  expected<std::exception_ptr, Oracle> e(v);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK(! ! e) ;
  BOOST_CHECK(e.valid());
  BOOST_CHECK(bool(e));
  BOOST_CHECK_EQUAL(e.value().s,  sMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sValueConstructed);

  expected<std::exception_ptr, Oracle> e2(std::move(v));
  BOOST_REQUIRE_NO_THROW(e2.value());
  BOOST_CHECK(! ! e2) ;
  BOOST_CHECK(e2.valid());
  BOOST_CHECK(bool(e2));
  BOOST_CHECK_EQUAL(e.value().s, sMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sMovedFrom);

}

BOOST_AUTO_TEST_CASE(expected_from_in_place_value)
{
  OracleVal v;
  expected<std::exception_ptr, Oracle> e{in_place_t{}, v};
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK(! ! e) ;
  BOOST_CHECK(e.valid());
  BOOST_CHECK(bool(e));
  BOOST_CHECK_EQUAL(e.value().s, sValueCopyConstructed);
  BOOST_CHECK_EQUAL(v.s, sValueConstructed);

  expected<std::exception_ptr, Oracle> e2{in_place_t{}, std::move(v)};
  BOOST_REQUIRE_NO_THROW(e2.value());
  BOOST_CHECK(! ! e2) ;
  BOOST_CHECK(e2.valid());
  BOOST_CHECK(bool(e2));
  BOOST_CHECK_EQUAL(e2.value().s, sValueMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sMovedFrom);

}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From unexpected_type constructor.
  expected<std::exception_ptr, int> e(make_unexpected(test_exception()));
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_copy_value)
{
  // From copy constructor.
  expected<std::exception_ptr, int> ef(5);
  expected<std::exception_ptr, int> e(ef);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_copy_exception)
{
  // From unexpected_type constructor.
  expected<std::exception_ptr, int> ef(make_unexpected(test_exception()));
  expected<std::exception_ptr, int> e(ef);
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_in_place)
{
  // From in_place2 constructor.
  expected<std::exception_ptr, std::string> e(in_place2, "in_place2");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  expected<std::exception_ptr, int> e(make_unexpected(std::make_exception_ptr(test_exception())));
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_moved_value)
{
  // From move value constructor.
  std::string value = "my value";
  expected<std::exception_ptr, std::string> e = std::move(value);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "my value");
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
    expected<std::exception_ptr, int> e(make_unexpected(std::current_exception()));

    BOOST_REQUIRE_THROW(e.value(), std::exception);
    BOOST_CHECK_EQUAL(e.valid(), false);
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(error_expected_constructors)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<std::error_condition, int> e(5);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From unexpected_type constructor.
  expected<std::error_condition, int> e(unexpected_type<std::error_condition>(std::make_error_condition(std::errc::invalid_argument)));
  auto error_from_except_check = [](const bad_expected_access<std::error_condition>& except)
  {
    return std::errc(except.error().value()) == std::errc::invalid_argument;
  };
  BOOST_REQUIRE_EXCEPTION(e.value(), bad_expected_access<std::error_condition>, error_from_except_check);
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
  BOOST_CHECK(!b);
}
BOOST_AUTO_TEST_CASE(except_value_constexpr)
{
  // From value constructor.
  BOOST_CONSTEXPR expected<int,int> e(1);
  BOOST_CONSTEXPR int x = e.value();
  BOOST_CHECK_EQUAL(x, 1);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE(except_expected_assignment)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  expected<std::exception_ptr, int> e(5);
  BOOST_CHECK_EQUAL(e.value(), 5);

  // From value assignment.
  e = 8;
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 8);
  BOOST_CHECK_EQUAL(*e, 8);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_copy_expected)
{
  expected<std::exception_ptr, int> e(5);
  expected<std::exception_ptr, int> e2(8);

  // From value assignment.
  e = e2;
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 8);
  BOOST_CHECK_EQUAL(*e, 8);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_moved_expected)
{
  expected<std::exception_ptr, std::string> e("e");
  expected<std::exception_ptr, std::string> e2("e2");

  // From value assignment.
  e = std::move(e2);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "e2");
  BOOST_CHECK_EQUAL(*e, "e2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));

  BOOST_REQUIRE_NO_THROW(e2.value());
  BOOST_CHECK_EQUAL(e2.value(), "");
  BOOST_CHECK_EQUAL(*e2, "");
  BOOST_CHECK(e2.valid());
  BOOST_CHECK(static_cast<bool>(e2));
}

BOOST_AUTO_TEST_CASE(expected_from_in_place)
{
  // From in_place2 constructor.
  expected<std::exception_ptr, std::string> e(in_place2, "in_place2");
  BOOST_CHECK_EQUAL(e.value(), "in_place2");

  // From emplace method.
  e.emplace("emplace method");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "emplace method");
  BOOST_CHECK_EQUAL(*e, "emplace method");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_move_value)
{
  expected<std::exception_ptr, std::string> e("v");

  std::string value = "my value";
  // From assignment operator.
  e = std::move(value);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "my value");
  BOOST_CHECK_EQUAL(*e, "my value");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_factories)

BOOST_AUTO_TEST_CASE(expected_from_in_place)
{
  // From in_place2 factory.
  //auto e = make_expected<std::exception_ptr, std::string>("in_place2");
  auto e = expected<std::exception_ptr, std::string>("in_place2");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_in_place_error)
{
  // From in_place2 factory.
  auto e = expected<std::error_condition, std::string>("in_place2");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "in_place2");
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
    expected<std::exception_ptr, int> e = make_unexpected(std::current_exception());

    BOOST_REQUIRE_THROW(e.value(), std::exception);
    BOOST_CHECK_EQUAL(e.valid(), false);
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
  }
}

//BOOST_AUTO_TEST_CASE(expected_from_error_catch_exception)
//{
//  // From catch block
//  try
//  {
//    throw test_exception();
//  }
//  catch(...)
//  {
//    auto throw_lambda = [](){ return make_expected_from_error<int,std::error_condition>();};
//
//    //BOOST_CHECK_THROW(throw_lambda(), test_exception);
//  }
//}


BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From unexpected_type constructor.
  auto e = make_expected_from_error<int>(std::make_error_condition(std::errc::invalid_argument));
  auto error_from_except_check = [](const bad_expected_access<std::error_condition>& except)
  {
    return std::errc(except.error().value()) == std::errc::invalid_argument;
  };
  BOOST_CHECK_EXCEPTION(e.value(), bad_expected_access<std::error_condition>, error_from_except_check);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From unexpected_type constructor.
  auto e = make_expected_from_exception<int>(test_exception());
  //auto e = expected<std::exception_ptr, int>(unexpected_type<>(test_exception()));
  BOOST_CHECK_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  auto e = expected<std::exception_ptr, int>(make_unexpected(test_exception()));
  BOOST_CHECK_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(make_expected_from_call_fun)
{
  BOOST_CHECK_NO_THROW(make_expected_from_call(throwing_fun));
  expected<std::exception_ptr, int> e = make_expected_from_call(throwing_fun);
  BOOST_CHECK_THROW(e.value(), std::exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);

  e = make_expected_from_call(nothrowing_fun);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 4);
  BOOST_CHECK_EQUAL(*e, 4);
  BOOST_CHECK_EQUAL(e.valid(), true);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), true);

#if 0
  BOOST_CHECK_THROW(make_expected_from_call<std::error_condition>(throwing_fun), test_exception);

  BOOST_CHECK_NO_THROW(make_expected_from_call<std::error_condition>(nothrowing_fun));
  expected<std::error_condition, int> e2 = make_expected_from_call<std::error_condition>(nothrowing_fun);
  BOOST_CHECK_NO_THROW(e2.value());
  BOOST_CHECK_EQUAL(e2.value(), 4);
  BOOST_CHECK_EQUAL(*e2, 4);
  BOOST_CHECK_EQUAL(e2.valid(), true);
  BOOST_CHECK_EQUAL(static_cast<bool>(e2), true);
#endif
}

BOOST_AUTO_TEST_CASE(make_expected_from_call_void_fun)
{
#if 0
  BOOST_CHECK_NO_THROW(make_expected_from_call(void_throwing_fun));
  expected<std::exception_ptr, void> e = make_expected_from_call(void_throwing_fun);
  BOOST_CHECK_THROW(e.value(), std::exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);

  e = make_expected_from_call(do_nothing_fun);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.valid(), true);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), true);

  BOOST_CHECK_THROW(make_expected_from_call<std::error_condition>(void_throwing_fun), test_exception);

  BOOST_CHECK_NO_THROW(make_expected_from_call<std::error_condition>(do_nothing_fun));
  expected<std::error_condition, void> e2 = make_expected_from_call<std::error_condition>(do_nothing_fun);
  BOOST_CHECK_NO_THROW(e2.value());
  BOOST_CHECK_EQUAL(e2.valid(), true);
  BOOST_CHECK_EQUAL(static_cast<bool>(e2), true);
#endif
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(error_expected_modifier)

BOOST_AUTO_TEST_CASE(expected_swap_value)
{
  // From value constructor.
  expected<std::exception_ptr, int> e(5);
  expected<std::exception_ptr, int> e2(8);

  e.swap(e2);

  BOOST_CHECK_EQUAL(e.value(), 8);
  BOOST_CHECK_EQUAL(e2.value(), 5);

  e2.swap(e);

  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(e2.value(), 8);
}

BOOST_AUTO_TEST_CASE(expected_swap_exception)
{
  // From value constructor.
  expected<std::exception_ptr, int> e = make_unexpected(std::invalid_argument("e"));
  expected<std::exception_ptr, int> e2 = make_unexpected(std::invalid_argument("e2"));

  e.swap(e2);

  auto equal_to_e = [](const std::invalid_argument& except) { return std::string(except.what()) == "e"; };
  auto equal_to_e2 = [](const std::invalid_argument& except) { return std::string(except.what()) == "e2"; };

  BOOST_CHECK_EXCEPTION(e.value(), std::invalid_argument, equal_to_e2);
  BOOST_CHECK_EXCEPTION(e2.value(), std::invalid_argument, equal_to_e);

  e2.swap(e);

  BOOST_CHECK_EXCEPTION(e.value(), std::invalid_argument, equal_to_e);
  BOOST_CHECK_EXCEPTION(e2.value(), std::invalid_argument, equal_to_e2);
}

BOOST_AUTO_TEST_CASE(expected_swap_function_value)
{
  // From value constructor.
  expected<std::exception_ptr, int> e(5);
  expected<std::exception_ptr, int> e2(8);

  swap(e, e2);

  BOOST_CHECK_EQUAL(e.value(), 8);
  BOOST_CHECK_EQUAL(e2.value(), 5);

  swap(e, e2);

  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(e2.value(), 8);
}

BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////
BOOST_AUTO_TEST_SUITE(expected_mbind)

BOOST_AUTO_TEST_CASE(expected_mbind)
{
  auto fun = [](bool b) -> expected<std::exception_ptr, int>
  {
    if(b)
      return make_expected(5);
    else
      return make_unexpected(test_exception());
  };

  auto add_five = [](int sum) -> int
  {
    return sum + 5;
  };

  auto launch_except = [](int sum) -> int
  {
    throw test_exception();
  };

  expected<std::exception_ptr, int> e = fun(true).mbind(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 10);

  e = fun(true).mbind(add_five).mbind(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 15);

  e = fun(false).mbind(add_five).mbind(add_five);
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).mbind(launch_except), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_mbind)
{
  auto fun = [](bool b)
  {
    if(b)
      return make_expected();
    else
      return expected<std::exception_ptr, void>(make_unexpected(test_exception()));
  };

  auto launch_except = []()
  {
    throw test_exception();
  };

  auto do_nothing = [](){};

  expected<std::exception_ptr, void> e = fun(true).mbind(do_nothing);
  BOOST_CHECK_NO_THROW(e.value());

  e = fun(false).mbind(do_nothing);
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).mbind(launch_except), test_exception);

}

BOOST_AUTO_TEST_SUITE_END()


////////////////////////////////////
BOOST_AUTO_TEST_SUITE(expected_then)

BOOST_AUTO_TEST_CASE(expected_non_void_then)
{
  auto fun = [](bool b) -> expected<std::exception_ptr, int>
  {
    if(b)
      return make_expected(5);
    else
      return make_unexpected(test_exception());
  };


  auto add_five = [](int sum) -> int
  {
    return sum + 5;
  };
  auto six = []() -> int
  {
    return 6;
  };

  auto pair = [](int a) -> bool
  {
    return (a%2)==0;
  };

  auto launch_except = [](int sum) -> int
  {
    throw test_exception();
  };

  expected<std::exception_ptr, int> e = fun(true).then(if_valued(add_five));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 10);

  e = fun(true).then(if_valued(ident(six)));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 6);

  e = fun(false).then(if_unexpected(ident(six)));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 6);

  expected<std::exception_ptr, bool> e1 = fun(true).then(if_valued(pair));
  BOOST_CHECK_NO_THROW(e1.value());
  BOOST_CHECK_EQUAL(*e1, false);



  e = fun(true).then(if_valued(add_five)).then(if_valued(add_five));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 15);

  e = fun(false).then(if_valued(add_five)).then(if_valued(add_five));
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).then(if_valued(launch_except)), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_then)
{
  auto fun = [](bool b) -> expected<std::exception_ptr, void>
  {
    if(b)
      return make_expected();
    else
      return make_unexpected(test_exception());
  };

  auto launch_except = []()
  {
    throw test_exception();
  };

  auto do_nothing = [](){};

  BOOST_CHECK(true);
  expected<std::exception_ptr, void> e = fun(true).then(if_valued(do_nothing));
  BOOST_CHECK_NO_THROW(e.value());

  e = fun(false).then(if_valued(do_nothing));
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).then(if_valued(launch_except)), test_exception);

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_recover)

BOOST_AUTO_TEST_CASE(expected_recover)
{
  auto fun = [](bool b)
  {
    if(b)
      return expected<std::exception_ptr, int>(5);
    else
      return expected<std::exception_ptr, int>(make_unexpected(test_exception()));
  };

  auto add_five = [](int sum) -> int
  {
    return sum + 5;
  };

  auto recover_error = [](std::exception_ptr p)
  {
    return make_expected(0);
  };

  auto recover_error_silent_failure = [](std::exception_ptr p)
  {
    return expected<std::exception_ptr, int>(make_unexpected(p));
  };

  auto recover_error_failure = [](std::exception_ptr p) -> expected<std::exception_ptr, int>
  {
    return expected<std::exception_ptr, int>(make_unexpected(test_exception()));
  };

  auto recover_error_throws = [](std::exception_ptr p) -> expected<std::exception_ptr, int>
  {
    throw test_exception();
  };

  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error).valid(), true);
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error).value(), 0);
  BOOST_CHECK_EQUAL(fun(true).catch_error(recover_error).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error_silent_failure).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error_failure).valid(), false);

  BOOST_CHECK_EQUAL(fun(true).mbind(add_five).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).mbind(add_five).catch_error(recover_error).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).mbind(add_five).catch_error(recover_error_silent_failure).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).mbind(add_five).catch_error(recover_error_failure).value(), 10);

  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error).mbind(add_five).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error).mbind(add_five).mbind(add_five).value(), 10);
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error_failure).mbind(add_five).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).mbind(add_five).catch_error(recover_error_failure).mbind(add_five).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).mbind(add_five).catch_error(recover_error_silent_failure).mbind(add_five).valid(), false);

  BOOST_CHECK_THROW(fun(false).catch_error(recover_error_throws), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_recover)
{
  auto fun = [](bool b)
  {
    if(b)
      return make_expected();
    else
      return expected<std::exception_ptr, void>(boost::make_unexpected(test_exception()));
  };

  auto do_nothing = [](){};

  auto recover_error = [](std::exception_ptr p)
  {
    return make_expected();
  };

  auto recover_error_silent_failure = [](std::exception_ptr p)
  {
    return expected<std::exception_ptr, void>(boost::make_unexpected(p));
  };

  auto recover_error_failure = [](std::exception_ptr p) -> expected<std::exception_ptr, void>
  {
    throw test_exception();
  };

  // The catch_error doesn't alter the expected if it's valid.
  BOOST_CHECK_EQUAL(fun(true).catch_error(recover_error_failure).valid(), true);

  // Simple catch_error tests.
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error).valid(), true);
  BOOST_CHECK_THROW(fun(false).catch_error(recover_error_failure), test_exception);
  BOOST_CHECK_EQUAL(fun(false).catch_error(recover_error_silent_failure).valid(), false);

  // With a mbind between.
  BOOST_CHECK_THROW(fun(false).mbind(do_nothing).catch_error(recover_error_failure), test_exception);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(proposal)

BOOST_AUTO_TEST_CASE(concept)
{
  using namespace std;

  {
  expected<string, int> ei = 0;
  expected<string, int> ej = 1;
  expected<string, int> ek = make_unexpected(string());

  ei = 1;
  ej = make_unexpected(string());;
  ek = 0;

  ei = make_unexpected(string());;
  ej = 0;
  ek = 1;
  }
}
BOOST_AUTO_TEST_CASE(init)
{
  using namespace std;
  {
    string s{"STR"};

    expected<int, string> ep{make_unexpected(-1)};              // unexpected value, requires Movable<E>
    expected<int, string> eq = {make_unexpected(-1)};           // unexpected value, requires Movable<E>

    expected<std::exception_ptr, string> es{s};                    // requires Copyable<T>
    expected<std::exception_ptr, string> et = s;                   // requires Copyable<T>
    expected<std::exception_ptr, string> ev = string{"STR"};       // requires Movable<T>

    expected<std::exception_ptr, string> ew;                       // unexpected value
    expected<std::exception_ptr, string> ex{};                     // unexpected value
    expected<std::exception_ptr, string> ey = {};                  // unexpected value
    expected<std::exception_ptr, string> ez = expected<std::exception_ptr, string>{};  // unexpected value
  }

  {
    expected<int, Guard> eg;                        // unexpected value
    expected<int, Guard> eh{};                      // unexpected value
    expected<int, Guard> ei{in_place2};               // calls Guard{} in place
    expected<int, Guard> ej{in_place2, "arg"};        // calls Guard{"arg"} in place
  }

  {
    expected<string, int> ei{unexpect};               // unexpected value, calls string{} in place
    expected<string, int> ej{unexpect, "arg"};        // unexpected value, calls string{"arg"} in place
  }
}
BOOST_AUTO_TEST_CASE(make_unexpected_fact)
{
  using namespace std;
  {
    expected<int,string> opt1 = make_unexpected(1);
    expected<int,string> opt2 = {unexpect, 1};

    opt1 = make_unexpected(1);
    opt2 = {unexpect, 1};
  }
}
BOOST_AUTO_TEST_CASE(error_exception_ts)
{
  using namespace std;
  {
    expected<error_exception<std::error_code, std::system_error>, int > e =
        make_unexpected(make_error_code(errc::invalid_argument));
    BOOST_CHECK(e.error()==make_error_code(errc::invalid_argument));
    try {
      e.value();
      BOOST_CHECK (false);
    } catch (std::system_error const& ex) {

    } catch (...) {
      BOOST_CHECK (false);
    }
    expected<error_exception<std::error_code, std::system_error>, int > e2 = e.get_unexpected();
    BOOST_CHECK(e2.error()==make_error_code(errc::invalid_argument));
    try {
      e2.value();
      BOOST_CHECK (false);
    } catch (std::system_error const& ex) {

    } catch (...) {
      BOOST_CHECK (false);
    }

  }
}
BOOST_AUTO_TEST_CASE(ensured_read_ts)
{
  using namespace std;
  {
    ensured_read<int> e = make_ensured_read(1);
    BOOST_CHECK(e==1);
  }
  {
    ensured_read<int> e = make_ensured_read(1);
    unexpected_type<ensured_read<int>> ue1 = make_unexpected(std::move(e));
    BOOST_CHECK(ue1.value()==1);
  }
  {
    expected<ensured_read<int>, int > e = make_unexpected(make_ensured_read(1));
    BOOST_CHECK(e.error()==1);
  }
  {
    expected<ensured_read<int>, int > e {unexpect, 1};
    BOOST_CHECK(e.error()==1);
  }
  {
    ensured_read<std::exception_ptr> e = make_ensured_read(std::make_exception_ptr(1));
    BOOST_CHECK_THROW(std::rethrow_exception(e.value()), int);
  }
  {
    expected<ensured_read<std::exception_ptr>, int > e = make_unexpected(make_ensured_read(std::make_exception_ptr(1)));
    BOOST_CHECK_THROW(std::rethrow_exception(e.error().value()), int);
  }
}
BOOST_AUTO_TEST_CASE(relational_operators)
{
  using namespace std;
  {
    expected<int, unsigned> e0{0};
    expected<int, unsigned> e1{1};
    expected<int, unsigned> eN{unexpect, -1};

    BOOST_CHECK (eN < e0);
    BOOST_CHECK (e0 < e1);
    BOOST_CHECK (! (e0 < eN));
    BOOST_CHECK (eN <= e0);
    BOOST_CHECK (e0 <= e1);

    BOOST_CHECK (e0 > eN);
    BOOST_CHECK (e1 > e0);
    BOOST_CHECK (e0 >= eN);
    BOOST_CHECK (e1 >= e0);

    BOOST_CHECK (!(eN  < eN));
    BOOST_CHECK (!(e1 < e1));
    BOOST_CHECK (eN <= eN);
    BOOST_CHECK (e1 <= e1);

    BOOST_CHECK (eN != e0);
    BOOST_CHECK (e0 != e1);
    BOOST_CHECK (eN == eN);
    BOOST_CHECK (e0 == e0);

    //////

    BOOST_CHECK (eN == make_unexpected(-1));
    BOOST_CHECK (e0 != make_unexpected(1));
    BOOST_CHECK (eN != 1u);
    BOOST_CHECK (e1 == 1u);

    BOOST_CHECK (eN < 1u);
    BOOST_CHECK (eN <= 1u);
    BOOST_CHECK (1u > eN);
    BOOST_CHECK (!(1u < eN));
    BOOST_CHECK (1u >= eN);
    BOOST_CHECK (make_unexpected(1) < e0);
    BOOST_CHECK (make_unexpected(1) <= e0);
    BOOST_CHECK (! (make_unexpected(1) > e0));
    BOOST_CHECK (! (make_unexpected(1) >= e0));


    BOOST_CHECK (!(e0 < make_unexpected(1)));
    BOOST_CHECK (!(e0 <= make_unexpected(1)));
    BOOST_CHECK (e0 > make_unexpected(1));
    BOOST_CHECK (e0 >= make_unexpected(1));

  }
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(movesem)
//////////////////////////////
BOOST_AUTO_TEST_CASE(moved_from_state)
{
  // first, test mock:
  MoveAware<int> i{1}, j{2};
  BOOST_CHECK (i.val == 1);
  BOOST_CHECK (!i.moved);
  BOOST_CHECK (j.val == 2);
  BOOST_CHECK (!j.moved);

  MoveAware<int> k = std::move(i);
  BOOST_CHECK (k.val == 1);
  BOOST_CHECK (!k.moved);
  BOOST_CHECK (i.val == 1);
  BOOST_CHECK (i.moved);

  k = std::move(j);
  BOOST_CHECK (k.val == 2);
  BOOST_CHECK (!k.moved);
  BOOST_CHECK (j.val == 2);
  BOOST_CHECK (j.moved);

  // now, test expected
  expected<std::exception_ptr, MoveAware<int>> oi{1}, oj{2};
  BOOST_CHECK (oi);
  BOOST_CHECK (!oi->moved);
  BOOST_CHECK (oj);
  BOOST_CHECK (!oj->moved);

  expected<std::exception_ptr, MoveAware<int>> ok = std::move(oi);
  BOOST_CHECK (ok);
  BOOST_CHECK (!ok->moved);
  BOOST_CHECK (oi);
  BOOST_CHECK (oi->moved);

  ok = std::move(oj);
  BOOST_CHECK (ok);
  BOOST_CHECK (!ok->moved);
  BOOST_CHECK (oj);
  BOOST_CHECK (oj->moved);
}
BOOST_AUTO_TEST_CASE(copy_move_ctor_optional_int)
{
  expected<std::exception_ptr, int> oi;
  expected<std::exception_ptr, int> oj = oi;

  BOOST_CHECK (!oj);
  BOOST_CHECK (oj == oi);
  BOOST_CHECK (!bool(oj));

  oi = 1;
  expected<std::exception_ptr, int> ok = oi;
  BOOST_CHECK (!!ok);
  BOOST_CHECK (bool(ok));
  BOOST_CHECK (ok == oi);
  BOOST_CHECK (ok != oj);
  BOOST_CHECK (*ok == 1);

  expected<std::exception_ptr, int> ol = std::move(oi);
  BOOST_CHECK (!!ol);
  BOOST_CHECK (bool(ol));
  BOOST_CHECK (ol == oi);
  BOOST_CHECK (ol != oj);
  BOOST_CHECK (*ol == 1);
}
BOOST_AUTO_TEST_CASE(expected_expected)
{
  expected<std::exception_ptr, expected<std::exception_ptr, int>> oi1 = make_unexpected(-1);
  BOOST_CHECK (!oi1);

  {
  expected<std::exception_ptr, expected<std::exception_ptr, int>> oi2 {expect};
  BOOST_CHECK (bool(oi2));
  BOOST_CHECK (!(*oi2));
  //std::cout << typeid(**oi2).name() << std::endl;
  }

  {
  expected<std::exception_ptr, expected<std::exception_ptr, int>> oi2 {expect, make_unexpected(-1)};
  BOOST_CHECK (bool(oi2));
  BOOST_CHECK (!*oi2);
  }

  {
  expected<std::exception_ptr, expected<std::exception_ptr, int>> oi2 {expected<std::exception_ptr, int>{}};
  BOOST_CHECK (bool(oi2));
  BOOST_CHECK (!*oi2);
  }

  expected<std::exception_ptr, int> oi;
  auto ooi = make_expected(oi);
  static_assert( std::is_same<expected<std::exception_ptr, expected<std::exception_ptr, int>>, decltype(ooi)>::value, "");

}
BOOST_AUTO_TEST_SUITE_END()

void process(){}
void process(int ){}
void processNil(){}

BOOST_AUTO_TEST_SUITE(Examples)
//////////////////////////////

BOOST_AUTO_TEST_CASE(example1)
{
  expected<std::exception_ptr, int> oi; // create disengaged object
  expected<std::exception_ptr, int> oj = {unexpect}; // alternative syntax
  oi = oj; // assign disengaged object
  expected<std::exception_ptr, int> ok = oj; // ok is disengaged

  if (oi) BOOST_CHECK(false); // 'if oi is engaged...'
  if (!oi) BOOST_CHECK(true); // 'if oi is disengaged...'

  BOOST_CHECK(oi == ok); // two disengaged optionals compare equal

  ///////////////////////////////////////////////////////////////////////////
  expected<std::exception_ptr, int> ol{1}; // ol is engaged; its contained value is 1
  ok = 2; // ok becomes engaged; its contained value is 2
  oj = ol; // oj becomes engaged; its contained value is 1

  BOOST_CHECK(oi != ol); // disengaged != engaged
  BOOST_CHECK(ok != ol); // different contained values
  BOOST_CHECK(oj == ol); // same contained value
  //BOOST_CHECK(oi < ol); // disengaged < engaged
  //BOOST_CHECK(ol < ok); // less by contained value

  /////////////////////////////////////////////////////////////////////////////
  expected<std::exception_ptr, int> om{1}; // om is engaged; its contained value is 1
  expected<std::exception_ptr, int> on = om; // on is engaged; its contained value is 1
  om = 2; // om is engaged; its contained value is 2
  BOOST_CHECK (on != om); // on still contains 3. They are not pointers

  /////////////////////////////////////////////////////////////////////////////
  int i = *ol; // i obtains the value contained in ol
  BOOST_CHECK (i == 1);
  *ol = 9; // the object contained in ol becomes 9
  BOOST_CHECK(*ol == 9);
  BOOST_CHECK(ol == make_expected(9));

  ///////////////////////////////////
  int p = 1;
  expected<std::exception_ptr, int> op = p;
  BOOST_CHECK(*op == 1);
  p = 2;
  BOOST_CHECK(*op == 1); // value contained in op is separated from p

  ////////////////////////////////
  if (ol)
    process(*ol); // use contained value if present
  else
    process(); // proceed without contained value

  if (!om)
    processNil();
  else
    process(*om);

  /////////////////////////////////////////
  process(ol.value_or(0)); // use 0 if ol is disengaged

  ////////////////////////////////////////////
  ok = {unexpect}; // if ok was engaged calls T's dtor
  oj = {}; // assigns a temporary disengaged expected
}
//////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(ValueOr)
{
  expected<std::exception_ptr, int> oi = 1;
  int i = oi.value_or(0);
  BOOST_CHECK (i == 1);

  oi = {unexpect};
  BOOST_CHECK (oi.value_or(3) == 3);

  expected<std::exception_ptr, std::string> os{"AAA"};
  BOOST_CHECK (os.value_or("BBB") == "AAA");
  os = {};
  BOOST_CHECK (os.value_or("BBB") == "BBB");
}

//////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE_END()


