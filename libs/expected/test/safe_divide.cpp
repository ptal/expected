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

#ifdef exception_based
int safe_divide(int i, int j)
{
  if (j==0) throw (DivideByZero());
  else return i/j;
}

int f(int i, int j, int k)
{
  return safe_divide(i,k) + safe_divide(j,k);
}
#endif

boost::expected<int> safe_divide(int i, int j)
{
  if (j == 0)
    return make_unexpected(DivideByZero());
  else
    return i / j;
}

#ifdef expect
boost::expected<int> ex_safe3_f(int i, int j, int k)
{
  return expect safe_divide(i,k) + expect safe_divide(j,k);
}

boost::expected<int> ex_safe3_f(int i, int j, int k) ensured
{
  auto s1 = expect safe_divide(i,k);
  auto s2 = expect safe_divide(j,k);
  return s1 + s2;
}
#endif

boost::expected<int> safe_f(int i, int j, int k)
{
  auto eq1 = safe_divide(i, k);
  if (!eq1.valid()) return eq1.get_exceptional();
  auto q1 = eq1.value();

  auto eq2 = safe_divide(j, k);
  if (!eq2.valid()) return eq2.get_exceptional();
  auto q2 = eq2.value();

  return q1 + q2;
}

#define EXPECT(V, EXPR) \
auto BOOST_JOIN(expected,V) = EXPR; \
if (! BOOST_JOIN(expected,V).valid()) return BOOST_JOIN(expected,V).get_exceptional(); \
auto V =BOOST_JOIN(expected,V).value()

boost::expected<int> msafe_f(int i, int j, int k)
{
  EXPECT(q1, safe_divide(i,k));
  EXPECT(q2, safe_divide(j,k));
  return q1 + q2;
}

//boost::expected<int> safe_f(int i, int j, int k) {
//  auto eq1 = safe_divide(i,k);
//  if (! eq1.valid()) return eq1.get_exceptional();
//  auto q1 = eq1.value();
//  auto eq2 = safe_divide(j,k);
//  if (! eq2.valid()) return eq2.get_exceptional();
//  auto q2 = eq2.value();
//  return q1 + q2;
//}


boost::expected<int> safe2_f(int i, int j, int k)
{
  auto q1 = safe_divide(i, k);
  return q1.then([j,k](int q1)
    {
      auto q2 = safe_divide(j,k);
      return q2.then([q1](int q2)
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
//  return i.then([j](int i) {
//    return j.then([i](int j) {
//      return i+j;
//    });
//  });
//}

boost::expected<int> safe3_f(int i, int j, int k)
{
  return safe_divide(i, k) + safe_divide(j, k);
}

#ifdef exception_based
T divide(T i, T j)
{
  try
  {
    return safe_divide(i,j)
  }
  catch(NotDivisible& ex)
  {
    return ex.i/ex.j;
  }
  catch(...)
  {
    throw;
  }
}
#endif

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

int main()
{
  auto r = safe_divide(1, 0);
  auto r1 = safe_f(1, 2, 0);
  auto mr1 = msafe_f(1, 2, 0);
  auto r2 = safe2_f(1, 2, 0);
  auto r3 = safe3_f(1, 2, 0);
  auto a = divide(1, 0);
  auto a2 = divide2(1, 0);
  return 0;
}

