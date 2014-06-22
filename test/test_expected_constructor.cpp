//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

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

#include <boost/expected/expected.hpp>

using namespace boost;

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(Expected_DefaultConstructor)
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<T>::expected()
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_DefaultConstructor_CopyableValueType_DefaultError)
{
  expected<int> ei;
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::exception_ptr());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<void>::expected()
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_DefaultConstructor_VoidValueType_DefaultError)
{
  expected<void> ei;
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::exception_ptr());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<E, T>::expected()
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_DefaultConstructor_CopyableValueType_UdtError)
{
  expected<int, std::string> ei;
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::string());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<void,E>::expected()
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_DefaultConstructor_VoidValueType_UdtError)
{
  expected<void, std::string> ei;
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::string());
}
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(Expected_ConstructorFromUnexpect)
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<T>::expected(unexpect_t, T ...)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpect_CopyableValueType_DefaultError)
{
  expected<int> ei{unexpect};
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::exception_ptr());
}
//BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpect1_CopyableValueType_DefaultError)
//{
//  expected<int> ei(unexpect, 1);
//  BOOST_CHECK (!ei);
//  BOOST_CHECK (ei.has_exception<int>());
//  //BOOST_CHECK (ei.error()==std::make_exception_ptr(1));
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<void>::expected(unexpect_t)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpect_VoidValueType_DefaultError)
{
  expected<void> ei{unexpect};
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::exception_ptr());
}
//BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpect1_VoidValueType_DefaultError)
//{
//  expected<void> ei(unexpect, 1);
//  BOOST_CHECK (!ei);
//  BOOST_CHECK (ei.has_exception<int>());
//  //BOOST_CHECK (ei.error()==std::make_exception_ptr(1));
//}
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<E, T>::expected(unexpect_t)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpect_CopyableValueType_UdtError)
{
  expected<int, std::string> ei{unexpect};
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::string());
}
BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpectA_CopyableValueType_UdtError)
{
  expected<int, std::string> ei(unexpect, "A");
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::string("A"));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// expected<void,E>::expected(unexpect_t)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpect_VoidValueType_UdtError)
{
  expected<void, std::string> ei{unexpect};
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::string());
}
BOOST_AUTO_TEST_CASE(Expected_ConstructorFromUnexpectA_VoidValueType_UdtError)
{
  expected<void, std::string> ei{unexpect, "A"};
  BOOST_CHECK (!ei);
  BOOST_CHECK (ei.error()==std::string("A"));
}
BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
