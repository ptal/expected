//! \file test_expected.cpp

// Copyright Pierre Talbot 2013.
// Copyright Vicente J. Botet Escriba 2013.

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
#include <boost/functional/adaptor.hpp>

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
  expected<int> e {};
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
  expected<int> e;
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
  expected<int> e(5);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_value2)
{
  // From value constructor.
  expected<int> e(5);
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
  expected<Oracle> e(v);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK(! ! e) ;
  BOOST_CHECK(e.valid());
  BOOST_CHECK(bool(e));
  BOOST_CHECK_EQUAL(e.value().s,  sMoveConstructed);
  BOOST_CHECK_EQUAL(v.s, sValueConstructed);

  expected<Oracle> e2(std::move(v));
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
  expected<Oracle> e{in_place_t{}, v};
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK(! ! e) ;
  BOOST_CHECK(e.valid());
  BOOST_CHECK(bool(e));
  BOOST_CHECK_EQUAL(e.value().s, sValueCopyConstructed);
  BOOST_CHECK_EQUAL(v.s, sValueConstructed);

  expected<Oracle> e2{in_place_t{}, std::move(v)};
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
  expected<int> e(make_unexpected(test_exception()));
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_copy_value)
{
  // From copy constructor.
  expected<int> ef(5);
  expected<int> e(ef);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_copy_exception)
{
  // From unexpected_type constructor.
  expected<int> ef(make_unexpected(test_exception()));
  expected<int> e(ef);
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_in_place)
{
  // From in_place2 constructor.
  expected<std::string> e(in_place2, "in_place2");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  expected<int> e(make_unexpected(std::make_exception_ptr(test_exception())));
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_moved_value)
{
  // From move value constructor.
  std::string value = "my value";
  expected<std::string> e = std::move(value);
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
    expected<int> e(make_unexpected(std::current_exception()));

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
  expected<int, std::error_condition> e(5);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From unexpected_type constructor.
  expected<int, std::error_condition> e(unexpected_type<std::error_condition>(std::make_error_condition(std::errc::invalid_argument)));
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
  expected<int> e(5);
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
  expected<int> e(5);
  expected<int> e2(8);

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
  expected<std::string> e("e");
  expected<std::string> e2("e2");

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
  expected<std::string> e(in_place2, "in_place2");
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
  expected<std::string> e("v");

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
  auto e = make_expected<std::string>("in_place2");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "in_place2");
  BOOST_CHECK_EQUAL(*e, "in_place2");
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
}

BOOST_AUTO_TEST_CASE(expected_from_in_place_error)
{
  // From in_place2 factory.
  auto e = expected<std::string, std::error_condition>("in_place2");
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
    expected<int> e = make_unexpected(std::current_exception());

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
  //auto e = expected<int>(unexpected_type<>(test_exception()));
  BOOST_CHECK_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  auto e = expected<int>(make_unexpected(test_exception()));
  BOOST_CHECK_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(make_expected_from_call_fun)
{
  BOOST_CHECK_NO_THROW(make_expected_from_call(throwing_fun));
  expected<int> e = make_expected_from_call(throwing_fun);
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
  expected<int, std::error_condition> e2 = make_expected_from_call<std::error_condition>(nothrowing_fun);
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
  expected<void> e = make_expected_from_call(void_throwing_fun);
  BOOST_CHECK_THROW(e.value(), std::exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);

  e = make_expected_from_call(do_nothing_fun);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.valid(), true);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), true);

  BOOST_CHECK_THROW(make_expected_from_call<std::error_condition>(void_throwing_fun), test_exception);

  BOOST_CHECK_NO_THROW(make_expected_from_call<std::error_condition>(do_nothing_fun));
  expected<void, std::error_condition> e2 = make_expected_from_call<std::error_condition>(do_nothing_fun);
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
  expected<int> e(5);
  expected<int> e2(8);

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
  expected<int> e = make_unexpected(std::invalid_argument("e"));
  expected<int> e2 = make_unexpected(std::invalid_argument("e2"));

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
  expected<int> e(5);
  expected<int> e2(8);

  swap(e, e2);

  BOOST_CHECK_EQUAL(e.value(), 8);
  BOOST_CHECK_EQUAL(e2.value(), 5);

  swap(e, e2);

  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(e2.value(), 8);
}

BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////
BOOST_AUTO_TEST_SUITE(expected_next)

BOOST_AUTO_TEST_CASE(expected_next)
{
  auto fun = [](bool b) -> expected<int>
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

  expected<int> e = fun(true).next(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 10);

  e = fun(true).next(add_five).next(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 15);

  e = fun(false).next(add_five).next(add_five);
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).next(launch_except), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_next)
{
  auto fun = [](bool b)
  {
    if(b)
      return make_expected();
    else
      return expected<void>(make_unexpected(test_exception()));
  };

  auto launch_except = []()
  {
    throw test_exception();
  };

  auto do_nothing = [](){};

  expected<void> e = fun(true).next(do_nothing);
  BOOST_CHECK_NO_THROW(e.value());

  e = fun(false).next(do_nothing);
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).next(launch_except), test_exception);

}

BOOST_AUTO_TEST_SUITE_END()


////////////////////////////////////
BOOST_AUTO_TEST_SUITE(expected_then)

BOOST_AUTO_TEST_CASE(expected_non_void_then)
{
  auto fun = [](bool b) -> expected<int>
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

  expected<int> e = fun(true).then(if_valued(add_five));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 10);

  e = fun(true).then(if_valued(ident(six)));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 6);

  e = fun(false).then(if_unexpected(ident(six)));
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 6);

  expected<bool> e1 = fun(true).then(if_valued(pair));
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
  auto fun = [](bool b) -> expected<void>
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
  expected<void> e = fun(true).then(if_valued(do_nothing));
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
      return expected<int>(5);
    else
      return expected<int>(make_unexpected(test_exception()));
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
    return expected<int>(make_unexpected(p));
  };

  auto recover_error_failure = [](std::exception_ptr p) -> expected<int>
  {
    return expected<int>(make_unexpected(test_exception()));
  };

  auto recover_error_throws = [](std::exception_ptr p) -> expected<int>
  {
    throw test_exception();
  };

  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).valid(), true);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).value(), 0);
  BOOST_CHECK_EQUAL(fun(true).recover(recover_error).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_silent_failure).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_failure).valid(), false);

  BOOST_CHECK_EQUAL(fun(true).next(add_five).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).next(add_five).recover(recover_error).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).next(add_five).recover(recover_error_silent_failure).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).next(add_five).recover(recover_error_failure).value(), 10);

  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).next(add_five).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).next(add_five).next(add_five).value(), 10);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_failure).next(add_five).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).next(add_five).recover(recover_error_failure).next(add_five).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).next(add_five).recover(recover_error_silent_failure).next(add_five).valid(), false);

  BOOST_CHECK_THROW(fun(false).recover(recover_error_throws), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_recover)
{
  auto fun = [](bool b)
  {
    if(b)
      return make_expected();
    else
      return expected<void>(boost::make_unexpected(test_exception()));
  };

  auto do_nothing = [](){};

  auto recover_error = [](std::exception_ptr p)
  {
    return make_expected();
  };

  auto recover_error_silent_failure = [](std::exception_ptr p)
  {
    return expected<void>(boost::make_unexpected(p));
  };

  auto recover_error_failure = [](std::exception_ptr p) -> expected<void>
  {
    throw test_exception();
  };

  // The recover doesn't alter the expected if it's valid.
  BOOST_CHECK_EQUAL(fun(true).recover(recover_error_failure).valid(), true);

  // Simple recover tests.
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).valid(), true);
  BOOST_CHECK_THROW(fun(false).recover(recover_error_failure), test_exception);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_silent_failure).valid(), false);

  // With a next between.
  BOOST_CHECK_THROW(fun(false).next(do_nothing).recover(recover_error_failure), test_exception);

}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(proposal)

BOOST_AUTO_TEST_CASE(concept)
{
  using namespace std;

  {
  expected<int, string> ei = 0;
  expected<int, string> ej = 1;
  expected<int, string> ek = make_unexpected(string());

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

    expected<string,int> ep{make_unexpected(-1)};              // unexpected value, requires Movable<E>
    expected<string,int> eq = {make_unexpected(-1)};           // unexpected value, requires Movable<E>

    expected<string> es{s};                    // requires Copyable<T>
    expected<string> et = s;                   // requires Copyable<T>
    expected<string> ev = string{"STR"};       // requires Movable<T>

    expected<string> ew;                       // unexpected value
    expected<string> ex{};                     // unexpected value
    expected<string> ey = {};                  // unexpected value
    expected<string> ez = expected<string>{};  // unexpected value
  }

  {
    expected<Guard, int> eg;                        // unexpected value
    expected<Guard, int> eh{};                      // unexpected value
    expected<Guard, int> ei{in_place2};               // calls Guard{} in place
    expected<Guard, int> ej{in_place2, "arg"};        // calls Guard{"arg"} in place
  }

  {
    expected<int,string> ei{unexpect};               // unexpected value, calls string{} in place
    expected<int,string> ej{unexpect, "arg"};        // unexpected value, calls string{"arg"} in place
  }
}
BOOST_AUTO_TEST_CASE(make_unexpected_fact)
{
  using namespace std;
  {
    expected<string,int> opt1 = make_unexpected(1);
    expected<string,int> opt2 = {unexpect, 1};

    opt1 =   make_unexpected(1);
    opt2 =  {unexpect, 1};
  }
}
BOOST_AUTO_TEST_CASE(error_exception_ts)
{
  using namespace std;
  {
    expected<int, error_exception<std::error_code, std::system_error> > e =
        make_unexpected(make_error_code(errc::invalid_argument));
    BOOST_CHECK(e.error()==make_error_code(errc::invalid_argument));
    try {
      e.value();
      BOOST_CHECK (false);
    } catch (std::system_error const& ex) {

    } catch (...) {
      BOOST_CHECK (false);
    }
    expected<int, error_exception<std::error_code, std::system_error> > e2 = e.get_unexpected();
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
BOOST_AUTO_TEST_CASE(relational_operators)
{
  using namespace std;
  {
    expected<unsigned, int> e0{0};
    expected<unsigned, int> e1{1};
    expected<unsigned, int> eN{unexpect, -1};

    BOOST_CHECK (eN < e0);
    BOOST_CHECK (e0 < e1);
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
  expected<MoveAware<int>> oi{1}, oj{2};
  BOOST_CHECK (oi);
  BOOST_CHECK (!oi->moved);
  BOOST_CHECK (oj);
  BOOST_CHECK (!oj->moved);

  expected<MoveAware<int>> ok = std::move(oi);
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
  expected<int> oi;
  expected<int> oj = oi;

  BOOST_CHECK (!oj);
  BOOST_CHECK (oj == oi);
  BOOST_CHECK (!bool(oj));

  oi = 1;
  expected<int> ok = oi;
  BOOST_CHECK (!!ok);
  BOOST_CHECK (bool(ok));
  BOOST_CHECK (ok == oi);
  BOOST_CHECK (ok != oj);
  BOOST_CHECK (*ok == 1);

  expected<int> ol = std::move(oi);
  BOOST_CHECK (!!ol);
  BOOST_CHECK (bool(ol));
  BOOST_CHECK (ol == oi);
  BOOST_CHECK (ol != oj);
  BOOST_CHECK (*ol == 1);
}
BOOST_AUTO_TEST_CASE(expected_expected)
{
  expected<expected<int>> oi1 = make_unexpected(-1);
  BOOST_CHECK (!oi1);

  {
  expected<expected<int>> oi2 {expect};
  BOOST_CHECK (bool(oi2));
  BOOST_CHECK (!(*oi2));
  //std::cout << typeid(**oi2).name() << std::endl;
  }

  {
  expected<expected<int>> oi2 {expect, make_unexpected(-1)};
  BOOST_CHECK (bool(oi2));
  BOOST_CHECK (!*oi2);
  }

  {
  expected<expected<int>> oi2 {expected<int>{}};
  BOOST_CHECK (bool(oi2));
  BOOST_CHECK (!*oi2);
  }

  expected<int> oi;
  auto ooi = make_expected(oi);
  static_assert( std::is_same<expected<expected<int>>, decltype(ooi)>::value, "");

}
BOOST_AUTO_TEST_SUITE_END()

void process(){}
void process(int ){}
void processNil(){}

BOOST_AUTO_TEST_SUITE(Examples)
//////////////////////////////

BOOST_AUTO_TEST_CASE(example1)
{
  expected<int> oi; // create disengaged object
  expected<int> oj = {unexpect}; // alternative syntax
  oi = oj; // assign disengaged object
  expected<int> ok = oj; // ok is disengaged

  if (oi) BOOST_CHECK(false); // 'if oi is engaged...'
  if (!oi) BOOST_CHECK(true); // 'if oi is disengaged...'

  BOOST_CHECK(oi == ok); // two disengaged optionals compare equal

  ///////////////////////////////////////////////////////////////////////////
  expected<int> ol{1}; // ol is engaged; its contained value is 1
  ok = 2; // ok becomes engaged; its contained value is 2
  oj = ol; // oj becomes engaged; its contained value is 1

  BOOST_CHECK(oi != ol); // disengaged != engaged
  BOOST_CHECK(ok != ol); // different contained values
  BOOST_CHECK(oj == ol); // same contained value
  //BOOST_CHECK(oi < ol); // disengaged < engaged
  //BOOST_CHECK(ol < ok); // less by contained value

  /////////////////////////////////////////////////////////////////////////////
  expected<int> om{1}; // om is engaged; its contained value is 1
  expected<int> on = om; // on is engaged; its contained value is 1
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
  expected<int> op = p;
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
BOOST_AUTO_TEST_CASE(BOOST_AUTO_TEST_CASE)
{
  expected<int> oi = 1;
  int i = oi.value_or(0);
  BOOST_CHECK (i == 1);

  oi = {unexpect};
  BOOST_CHECK (oi.value_or(3) == 3);

  expected<std::string> os{"AAA"};
  BOOST_CHECK (os.value_or("BBB") == "AAA");
  os = {};
  BOOST_CHECK (os.value_or("BBB") == "BBB");
};

BOOST_AUTO_TEST_SUITE_END()
///////////////////////////
BOOST_AUTO_TEST_SUITE(Void)
BOOST_AUTO_TEST_SUITE(DefaultConstructor)
BOOST_AUTO_TEST_CASE(BOOST_AUTO_TEST_CASE)
{
  expected<int> oi;
}
BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
