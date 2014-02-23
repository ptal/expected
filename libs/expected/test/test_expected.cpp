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

int throwing_fun(){ throw test_exception(); }
int nothrowing_fun(){ return 4; }

void void_throwing_fun(){ throw test_exception(); }
void do_nothing_fun(){}


BOOST_AUTO_TEST_SUITE(except_default_constructor)

BOOST_AUTO_TEST_CASE(except_default_constructor)
{
  // From value constructor.
  expected<int> e;
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK(e.valid());
  BOOST_CHECK(! ! e);
  BOOST_CHECK(static_cast<bool>(e));
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
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
  BOOST_CHECK(static_cast<bool>(e));
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
  // From unexpected constructor.
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
  // From unexpected constructor.
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
  // From unexpected constructor.
  expected<int, std::error_condition> e(unexpected<std::error_condition>(std::make_error_condition(std::errc::invalid_argument)));
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
  BOOST_CHECK(b);
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
    //expected<int> e = make_expected_from_error<int>();
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
  // From unexpected constructor.
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
  // From unexpected constructor.
  auto e = make_expected_from_error<int>(test_exception());
  BOOST_CHECK_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
}

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  auto e = make_expected_from_error<int>(std::make_exception_ptr(test_exception()));
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
      return expected<int>(5);
    else
      return make_expected_from_error<int>(std::make_exception_ptr(test_exception()));
  };

  auto add_five = [](int sum) -> int
  {
    return sum + 5;
  };

  auto launch_except = [](int sum) -> int
  {
    throw test_exception();
  };

  expected<int> e = fun(true).then(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 10);

  e = fun(true).then(add_five).then(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 15);

  e = fun(false).then(add_five).then(add_five);
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).then(launch_except), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_next)
{
  auto fun = [](bool b)
  {
    if(b)
      return expected<void>();
    else
      return make_expected_from_error<void>(std::make_exception_ptr(test_exception()));
  };

  auto launch_except = []()
  {
    throw test_exception();
  };

  auto do_nothing = [](){};

  expected<void> e = fun(true).then(do_nothing);
  BOOST_CHECK_NO_THROW(e.value());

  e = fun(false).then(do_nothing);
  BOOST_CHECK_THROW(e.value(), test_exception);

  BOOST_CHECK_THROW(fun(true).then(launch_except), test_exception);

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
      return make_expected_from_error<int>(std::make_exception_ptr(test_exception()));
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
    return make_expected_from_error<int>(p);
  };

  auto recover_error_failure = [](std::exception_ptr p) -> expected<int>
  {
    return make_expected_from_error<int>(test_exception());
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

  BOOST_CHECK_EQUAL(fun(true).then(add_five).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).then(add_five).recover(recover_error).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).then(add_five).recover(recover_error_silent_failure).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).then(add_five).recover(recover_error_failure).value(), 10);

  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).then(add_five).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).then(add_five).then(add_five).value(), 10);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_failure).then(add_five).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).then(add_five).recover(recover_error_failure).then(add_five).valid(), false);
  BOOST_CHECK_EQUAL(fun(false).then(add_five).recover(recover_error_silent_failure).then(add_five).valid(), false);

  BOOST_CHECK_THROW(fun(false).recover(recover_error_throws), test_exception);

}

BOOST_AUTO_TEST_CASE(expected_void_recover)
{
  auto fun = [](bool b)
  {
    if(b)
      return expected<void>();
    else
      return make_expected_from_error<void>(std::make_exception_ptr(test_exception()));
  };

  auto do_nothing = [](){};

  auto recover_error = [](std::exception_ptr p)
  {
    return expected<void>();
  };

  auto recover_error_silent_failure = [](std::exception_ptr p)
  {
    return make_expected_from_error<void>(p);
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

  // With a then between.
  BOOST_CHECK_THROW(fun(false).then(do_nothing).recover(recover_error_failure), test_exception);

}

BOOST_AUTO_TEST_SUITE_END()
