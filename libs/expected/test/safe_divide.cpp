//! \file safe_divide.cpp

// Copyright Vicente J. Botet Escriba 2013.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected.hpp>
#include <iostream>
#include <exception>

using namespace boost;

struct DivideByZero: public std::exception
{
};
struct NotDivisible: public std::exception
{
  int i, j;
};

namespace exception_based
{
  int safe_divide(int i, int j)
  {
    if (j == 0)
      throw (DivideByZero());
    else
      return i / j;
  }

  int f1(int i, int j, int k)
  {
    return i + safe_divide(j, k);
  }

  int f2(int i, int j, int k)
  {
    return safe_divide(i, k) + safe_divide(j, k);
  }
}

boost::expected<int> safe_divide(int i, int j)
{
  if (j == 0)
    return make_unexpected(DivideByZero());
  else
    return i / j;
}

#ifdef expect
boost::expected<int> ex_f1(int i, int j, int k)
{
  return i + expect safe_divide(j,k);
}
boost::expected<int> ex_f1(int i, int j, int k)
{
  auto q = expect safe_divide(j,k);
  return i + q;
}

boost::expected<int> ex_f2(int i, int j, int k)
{
  return expect safe_divide(i,k) + expect safe_divide(j,k);
}

boost::expected<int> ex_f2(int i, int j, int k) ensured
{
  auto s1 = expect safe_divide(i,k);
  auto s2 = expect safe_divide(j,k);
  return s1 + s2;
}
#endif

boost::expected<int> ex_f1(int i, int j, int k)
{
  auto eq = safe_divide(j, k);
  if (!eq.valid()) return eq.get_unexpected();
  auto q = *eq;

  return 1 + q;
}

boost::expected<int> ex_f2(int i, int j, int k)
{
  auto eq1 = safe_divide(i, k);
  if (!eq1.valid()) return eq1.get_unexpected();
  auto q1 = *eq1;

  auto eq2 = safe_divide(j, k);
  if (!eq2.valid()) return eq2.get_unexpected();
  auto q2 = *eq2;

  return q1 + q2;
}

#define EXPECT(V, EXPR) \
auto BOOST_JOIN(expected,V) = EXPR; \
if (! BOOST_JOIN(expected,V).valid()) return BOOST_JOIN(expected,V).get_unexpected(); \
auto V =*BOOST_JOIN(expected,V)

boost::expected<int> mex_f2(int i, int j, int k)
{
  EXPECT(q1, safe_divide(i,k));
  EXPECT(q2, safe_divide(j,k));
  return q1 + q2;
}

boost::expected<int> then_f2(int i, int j, int k)
{
  auto q1 = safe_divide(i, k);
  return q1.next([j,k](int q1)
    {
      auto q2 = safe_divide(j,k);
      return q2.next([q1](int q2)
          { return q1+q2;});
    });
}

template <class T>
expected<T> operator+(expected<T> i, expected<T> j)
{
  EXPECT(i_, i);
  EXPECT(j_, j);
  return i_ + j_;
}

//expected<int> operator+(expected<int> i, expected<int> j) {
//  return i.next([j](int i) {
//    return j.next([i](int j) {
//      return i+j;
//    });
//  });
//}

boost::expected<int> cex_f2(int i, int j, int k)
{
  return safe_divide(i, k) + safe_divide(j, k);
}

namespace exception_based
{
  int divide(int i, int j)
  {
    try
    {
      return safe_divide(i, j);
    }
    catch (NotDivisible& ex)
    {
      return ex.i / ex.j;
    }
    catch (...)
    {
      throw;
    }
  }
}

boost::expected<int> divide(int i, int j)
{
return safe_divide(i,j).
recover([](std::exception_ptr ex) -> boost::expected<int>
    {
      try
      {
        std::rethrow_exception(ex);
      }
      catch(DivideByZero& e)
      {
        return make_unexpected(e);
      }
      catch(NotDivisible& e)
      {
        return e.i / e.j;
      }
      catch (...)
      {
        return make_unexpected(std::current_exception());
      }
    });
}

boost::expected<int> divide2(int i, int j)
{
return safe_divide(i,j).
catch_exception<NotDivisible>([](NotDivisible& e) -> expected<int>
    {
      return e.i / e.j;
    });
}

boost::expected<int> divide3(int i, int j)
{
return safe_divide(i,j).
catch_exception<NotDivisible>([](NotDivisible& e)
    {
      return make_expected(e.i / e.j);
    });
}

int main()
{
  auto r = safe_divide(1, 0);
  auto r1 = ex_f1(1, 2, 0);
  auto r2 = ex_f2(1, 2, 0);
  auto mr2 = mex_f2(1, 2, 0);
  auto tr2 = then_f2(1, 2, 0);
  auto r3 = cex_f2(1, 2, 0);
  auto a = divide(1, 0);
  auto a2 = divide2(1, 0);
  return 0;
}

