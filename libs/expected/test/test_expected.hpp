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

#ifdef EXPECTED_CPP11_TESTS
  #define BOOST_RESULT_OF_USE_DECLTYPE
  #define BOOST_EXPECTED_USE_STD_EXCEPTION_PTR

  typedef std::exception_ptr exception_ptr_type;
#endif

#include "../../../boost/expected/expected.hpp"

#include <boost/bind.hpp>

#include <stdexcept>
#include <exception>

#ifdef EXPECTED_CPP11_TESTS
  #include <system_error>
  #define ERROR_CONDITION_NS std
#else
  #include <boost/system/error_code.hpp>
  #include <boost/system/system_error.hpp>
  #define ERROR_CONDITION_NS boost::system
#endif

using namespace boost;

class test_exception : public std::exception
{
};

int throwing_fun(){ throw test_exception();  }
int nothrowing_fun(){ return 4; }

void void_throwing_fun(){ throw test_exception();  }
void do_nothing_fun(){}

BOOST_AUTO_TEST_SUITE(except_expected_constructors)

#ifdef EXPECTED_CPP11_TESTS
BOOST_AUTO_TEST_CASE(expected_default_constructor)
{
  expected<int> e;
  BOOST_CHECK_EQUAL(e.valid(), false);
  BOOST_CHECK(e.error() == std::exception_ptr());

  class DefaultConstructibleTest
  {
    int s;
  public:
    DefaultConstructibleTest() = default;
    DefaultConstructibleTest(int i) : s(i){}

    int value() const { return s; }
  };

  expected<DefaultConstructibleTest> e2;
  BOOST_CHECK_EQUAL(e2.valid(), false);
  BOOST_CHECK(e2.error() == std::exception_ptr());
}
#endif

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<int> e(5);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}

BOOST_AUTO_TEST_CASE(expected_from_exception)
{
  // From unexpected_type constructor.
  expected<int> e(make_unexpected(test_exception()));
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
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
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}

BOOST_AUTO_TEST_CASE(expected_from_copy_exception)
{
  // From unexpected_type constructor.
  expected<int> ef(make_unexpected(test_exception()));
  expected<int> e(ef);
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
}

#ifdef EXPECTED_CPP11_TESTS
BOOST_AUTO_TEST_CASE(expected_from_emplace)
{
  // From emplace constructor.
  expected<std::string> e(in_place2, "emplace");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "emplace");
  BOOST_CHECK_EQUAL(*e, "emplace");
  BOOST_CHECK(e.valid());
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}
#endif

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  expected<int> e(make_unexpected(test_exception()));
  BOOST_REQUIRE_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
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
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
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
#ifdef EXPECTED_CPP11_TESTS
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(error_expected_constructors)

BOOST_AUTO_TEST_CASE(expected_from_value)
{
  // From value constructor.
  expected<int, ERROR_CONDITION_NS::error_condition> e(5);
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 5);
  BOOST_CHECK_EQUAL(*e, 5);
  BOOST_CHECK(e.valid());
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}

#ifdef EXPECTED_CPP11_TESTS
BOOST_AUTO_TEST_CASE(expected_from_error)
{
  // From unexpected_type constructor.
  expected<int, ERROR_CONDITION_NS::error_condition> e(unexpected_type<ERROR_CONDITION_NS::error_condition>(ERROR_CONDITION_NS::make_error_condition(ERROR_CONDITION_NS::errc::invalid_argument)));
  auto error_from_except_check = [](const bad_expected_access<ERROR_CONDITION_NS::error_condition>& except)
  {
    return ERROR_CONDITION_NS::errc(except.error().value()) == ERROR_CONDITION_NS::errc::invalid_argument;
  };
  BOOST_REQUIRE_EXCEPTION(e.value(), bad_expected_access<ERROR_CONDITION_NS::error_condition>, error_from_except_check);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
}
#endif

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
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
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
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}

#ifdef EXPECTED_CPP11_TESTS
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
#endif

BOOST_AUTO_TEST_CASE(expected_from_emplace)
{
  // From emplace constructor.
  expected<std::string> e(in_place2, "emplace");
  BOOST_CHECK_EQUAL(e.value(), "emplace");

  // From emplace method.
  e.emplace("emplace method");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "emplace method");
  BOOST_CHECK_EQUAL(*e, "emplace method");
  BOOST_CHECK(e.valid());
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}

#ifdef EXPECTED_CPP11_TESTS
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
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}
#endif

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_factories)

BOOST_AUTO_TEST_CASE(expected_from_emplace)
{
  // From emplace factory.
  boost::expected<std::string> e = make_expected<std::string>("emplace");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "emplace");
  BOOST_CHECK_EQUAL(*e, "emplace");
  BOOST_CHECK(e.valid());
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
}

BOOST_AUTO_TEST_CASE(expected_from_emplace_error)
{
  // From emplace factory.
  boost::expected<std::string, ERROR_CONDITION_NS::error_condition> e =
    expected<std::string, ERROR_CONDITION_NS::error_condition>("emplace");
  BOOST_REQUIRE_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), "emplace");
  BOOST_CHECK_EQUAL(*e, "emplace");
  BOOST_CHECK(e.valid());
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK(static_cast<bool>(e));
#endif
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
#ifdef EXPECTED_CPP11_TESTS
    BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
  }
}


//#ifdef EXPECTED_CPP11_TESTS
//BOOST_AUTO_TEST_CASE(expected_from_error_catch_exception)
//{
//  // From catch block
//  try
//  {
//    throw test_exception();
//  }
//  catch(...)
//  {
//    auto throw_lambda = [](){ return make_expected_from_error<int, ERROR_CONDITION_NS::error_condition>();};
//
//    //BOOST_CHECK_THROW(throw_lambda(), test_exception);
//  }
//}
//#endif

BOOST_AUTO_TEST_CASE(expected_from_exception_ptr)
{
  // From exception_ptr constructor.
  boost::expected<int> e = make_unexpected(test_exception());
  BOOST_CHECK_THROW(e.value(), test_exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif
}

BOOST_AUTO_TEST_CASE(expected_from_noexcept_fun)
{
  BOOST_CHECK_NO_THROW(make_expected_from_call(throwing_fun));
  expected<int> e = make_expected_from_call(throwing_fun);
  BOOST_CHECK_THROW(e.value(), std::exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif

  e = make_expected_from_call(nothrowing_fun);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.value(), 4);
  BOOST_CHECK_EQUAL(*e, 4);
  BOOST_CHECK_EQUAL(e.valid(), true);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), true);
#endif
}

BOOST_AUTO_TEST_CASE(expected_from_noexcept_void_fun)
{
  BOOST_CHECK_NO_THROW(make_expected_from_call(void_throwing_fun));
  expected<void> e = make_expected_from_call(void_throwing_fun);
  BOOST_CHECK_THROW(e.value(), std::exception);
  BOOST_CHECK_EQUAL(e.valid(), false);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), false);
#endif

  e = make_expected_from_call(do_nothing_fun);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(e.valid(), true);
#ifdef EXPECTED_CPP11_TESTS
  BOOST_CHECK_EQUAL(static_cast<bool>(e), true);
#endif
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_swap)

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

#ifdef EXPECTED_CPP11_TESTS
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
#endif

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

BOOST_AUTO_TEST_SUITE(expected_then)

#ifdef EXPECTED_CPP11_TESTS
BOOST_AUTO_TEST_CASE(expected_then)
{
  auto fun = [](bool b)
  {
    if(b)
      return expected<int>(5);
    else
      return expected<int>(boost::make_unexpected(test_exception()));
  };

  auto add_five = [](int sum)
  {
    return sum + 5;
  };

  expected<int> e = fun(true).next(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 10);

  e = fun(true).next(add_five).next(add_five);
  BOOST_CHECK_NO_THROW(e.value());
  BOOST_CHECK_EQUAL(*e, 15);

  e = fun(false).next(add_five).next(add_five);
  BOOST_CHECK_THROW(e.value(), test_exception);
}

BOOST_AUTO_TEST_CASE(expected_void_then)
{
  auto fun = [](bool b)
  {
    if(b)
      return make_expected();
    else
      return expected<void>(boost::make_unexpected(test_exception()));
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

  //e = fun(true).next(launch_except);
  //BOOST_CHECK_THROW(e.value(), std::exception);
}
#endif

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(expected_recover)

#ifdef EXPECTED_CPP11_TESTS
BOOST_AUTO_TEST_CASE(expected_recover)
{
  auto fun = [](bool b)
  {
    if(b)
      return expected<int>(5);
    else
      return expected<int>(boost::make_unexpected(test_exception()));
  };

  auto then_launch = [](int) -> int
  {
    throw test_exception();
  };

  auto add_five = [](int sum)
  {
    return sum + 5;
  };

  auto recover_error = [](exception_ptr_type p)
  {
    return 0;
  };

  auto recover_error_silent_failure = [](exception_ptr_type p)
  {
    return expected<int>(make_unexpected(p));
  };

  auto recover_error_failure = [](exception_ptr_type p) -> expected<int>
  {
    throw test_exception();
  };

  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).value(), 0);
  BOOST_CHECK_EQUAL(fun(true).recover(recover_error).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_silent_failure).valid(), false);
  BOOST_CHECK_THROW(fun(false).recover(recover_error_failure).valid(), test_exception);

  BOOST_CHECK_EQUAL(fun(true).next(add_five).recover(recover_error).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).next(add_five).recover(recover_error_silent_failure).value(), 10);
  BOOST_CHECK_EQUAL(fun(true).next(add_five).recover(recover_error_failure).value(), 10);

  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).next(add_five).value(), 5);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).next(add_five).next(add_five).value(), 10);
  BOOST_CHECK_THROW(fun(false).recover(recover_error_failure).next(add_five).valid(), test_exception);
  BOOST_CHECK_THROW(fun(false).next(add_five).recover(recover_error_failure).next(add_five).valid(), test_exception);
  BOOST_CHECK_EQUAL(fun(false).next(add_five).recover(recover_error_silent_failure).next(add_five).valid(), false);

  BOOST_CHECK_THROW(fun(true).next(then_launch).recover(recover_error).value(), test_exception);
  BOOST_CHECK_THROW(fun(true).next(then_launch).recover(recover_error).next(add_five).value(), test_exception);
  BOOST_CHECK_THROW(fun(true).next(then_launch).recover(recover_error_failure).valid(), test_exception);
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

  auto then_launch = []() -> void
  {
    throw test_exception();
  };

  auto do_nothing = [](){};

  auto recover_error = [](exception_ptr_type p)
  {
    return make_expected();
  };

  auto recover_error_silent_failure = [](exception_ptr_type p)
  {
    return expected<void>(boost::make_unexpected(p));
  };

  auto recover_error_failure = [](exception_ptr_type p) -> expected<void>
  {
    throw test_exception();
  };

  // The recover doesn't alter the expected if it's valid.
  BOOST_CHECK_EQUAL(fun(true).recover(recover_error_failure).valid(), true);

  // Simple recover tests.
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error).valid(), true);
  BOOST_CHECK_THROW(fun(false).recover(recover_error_failure).valid(), test_exception);
  BOOST_CHECK_EQUAL(fun(false).recover(recover_error_silent_failure).valid(), false);

  // With a next between.
  BOOST_CHECK_THROW(fun(false).next(do_nothing).recover(recover_error_failure).valid(), test_exception);

  BOOST_CHECK_NO_THROW(fun(false).next(then_launch).recover(recover_error).value());
}
#endif

BOOST_AUTO_TEST_SUITE_END()
