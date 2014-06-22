//! \file test_expected.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE "Expected Test Suite - Algorithm value_or"
#define BOOST_LIB_DIAGNOSTIC "on"// Show library file details.
// Linking to lib file: libboost_unit_test_framework-vc100-mt-gd-1_47.lib (trunk at 12 Jun 11)
#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected_monad.hpp>
#include <boost/functional/monads/categories/pointer_like.hpp>
#include <boost/functional/monads/algorithms/equal.hpp>
#include <utility>

#include <boost/test/unit_test.hpp> // Enhanced for unit_test framework autolink
#include <boost/test/included/unit_test.hpp>

using namespace boost;
//using namespace boost::functional;

bool equal(int i, int j) { return i==j; }
bool equal(int* i, int* j) { return i==j; }

////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_SUITE(Equal_Expected)
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Equal_Valued_Valued_eq)
{
  using namespace boost::functional::valued;

  expected<int> e1 = 1;
  expected<int> e2 = 1;
  BOOST_CHECK ( equal(e1, e2) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Equal_Valued_Valued_eq_ptr)
{
  int v1 = 1;
  int v2 = 1;
  BOOST_CHECK ( boost::functional::valued::equal(&v1, &v2) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Equal_Valued_Valued_neq)
{
  using namespace boost::functional::valued;

  expected<int> e1 = 1;
  expected<int> e2 = 2;
  BOOST_CHECK ( ! equal(e1, e2) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Equal_Valued_Unexpected)
{
  using namespace boost::functional::valued;

  expected<int> e1 = 1;
  expected<int> e2;
  BOOST_CHECK ( ! equal(e1, e2) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Equal_Unexpected_Valued)
{
  using namespace boost::functional::valued;

  expected<int> e1 = 1;
  expected<int> e2;
  BOOST_CHECK ( ! equal(e2, e1) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE(Equal_Unexpected_Unexpected)
{
  using namespace boost::functional::valued;

  expected<int> e1;
  expected<int> e2;
  BOOST_CHECK ( equal(e1, e2) );
  BOOST_CHECK ( equal(e2, e1) );
}
BOOST_AUTO_TEST_CASE(Equal_eq_int)
{
  using namespace boost::functional::valued;

  int e1 = 1;
  int e2 = 1;
  BOOST_CHECK ( equal(e1, e2) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()
////////////////////////////////////////////////////////////////////////////////////////////////////
