//! \file safe_divide.cpp

// Copyright Vicente J. Botet Escriba 2013.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/expected/expected_monad.hpp>
#include <boost/expected/optional_monad.hpp>
#include <boost/expected/conversions/expected_to_optional.hpp>
#include <boost/expected/conversions/expected_to_future.hpp>

#include <iostream>
#include <exception>

using namespace boost;
using namespace boost::monads;

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
namespace expected_based
{
  expected<int> safe_divide(int i, int j)
  {
    if (j == 0)
      return make_unexpected(DivideByZero());
    else
      return i / j;
  }
}
namespace optional_based
{
  optional<int> safe_divide(int i, int j)
  {
    if (j == 0)
      return none; // todo: try to make use of a generic make error
    else
      return i / j;
  }

}

#ifdef expect
expected<int> ex_f1(int i, int j, int k)
{
  return i + expect safe_divide(j,k);
}
expected<int> ex_f1(int i, int j, int k)
{
  auto q = expect safe_divide(j,k);
  return i + q;
}

expected<int> ex_f2(int i, int j, int k)
{
  return expect safe_divide(i,k) + expect safe_divide(j,k);
}

expected<int> ex_f2(int i, int j, int k) ensured
{
  auto s1 = expect safe_divide(i,k);
  auto s2 = expect safe_divide(j,k);
  return s1 + s2;
}
#endif

namespace expected_based
{
  expected<int> ex_f1(int i, int j, int k)
  {
    auto eq = safe_divide(j, k);
    if (! monads::has_value(eq)) return monads::get_unexpected(eq);
    auto q = monads::value_pre_has_value(eq);

    return 1 + q;
  }

  expected<int> ex_f2(int i, int j, int k)
  {
    auto eq1 = safe_divide(i, k);
    if (! monads::has_value(eq1)) return monads::get_unexpected(eq1);
    auto q1 = monads::value_pre_has_value(eq1);

    auto eq2 = safe_divide(j, k);
    if (! monads::has_value(eq2)) return monads::get_unexpected(eq2);
    auto q2 = monads::value_pre_has_value(eq2);

    return q1 + q2;
  }
}
namespace optional_based
{
  optional<int> ex_f1(int i, int j, int k)
  {
    auto eq = safe_divide(j, k);
    if (! monads::has_value(eq)) return monads::get_unexpected(eq);
    auto q = monads::value_pre_has_value(eq);

    return 1 + q;
  }

  optional<int> ex_f2(int i, int j, int k)
  {
    auto eq1 = safe_divide(i, k);
    if (!eq1) return monads::get_unexpected(eq1);
    auto q1 = monads::value_pre_has_value(eq1);

    auto eq2 = safe_divide(j, k);
    if (!eq2) return monads::get_unexpected(eq2);
    auto q2 = monads::value_pre_has_value(eq2);

    return q1 + q2;
  }
}
#define EXPECT(V, EXPR) \
auto BOOST_JOIN(expected,V) = EXPR; \
if (! boost::monads::has_value(BOOST_JOIN(expected,V))) return boost::monads::get_unexpected(BOOST_JOIN(expected,V)); \
auto V = boost::monads::value_pre_has_value(BOOST_JOIN(expected,V))

namespace expected_based
{
  expected<int> mex_f2(int i, int j, int k)
  {
    EXPECT(q1, safe_divide(i,k));
    EXPECT(q2, safe_divide(j,k));
    return q1 + q2;
  }
}
namespace optional_based
{
  optional<int> mex_f2(int i, int j, int k)
  {
    EXPECT(q1, safe_divide(i,k));
    EXPECT(q2, safe_divide(j,k));
    return q1 + q2;
  }
}
namespace expected_based
{
  expected<int> then_f22(int i, int j, int k)
  {
    return  mbind(safe_divide(i, k),
      [=](int q1)
      {
        return mbind(safe_divide(j,k), [=](int q2)
            {
              return q1+q2;
            });
      });
  }

  expected<int> then_f23(int i, int j, int k)
  {
    return fmap([=](int q1, int q2)
        { return q1+q2;},
        safe_divide(i, k), safe_divide(j, k));
  }

  expected<int> then_f2(int i, int j, int k)
  {
    return make_monad(safe_divide(i, k))
    & [=](int q1)
    {
      return expected<int>(make_monad(safe_divide(j,k)) & [=](int q2)
          {
            return q1+q2;
          });
    };
  }
}

namespace optional_based
{
  optional<int> then_f22(int i, int j, int k)
  {
    return  mbind(safe_divide(i, k),
      [=](int q1)
      {
        return mbind(safe_divide(j,k), [=](int q2)
            {
              return q1+q2;
            });
      });
  }

  optional<int> then_f23(int i, int j, int k)
  {
    return fmap([=](int q1, int q2)
        { return q1+q2;},
        safe_divide(i, k), safe_divide(j, k));
  }

  optional<int> then_f2(int i, int j, int k)
  {
    return make_monad(safe_divide(i, k))
    & [=](int q1)
    {
      return make_monad(safe_divide(j,k)).mbind([=](int q2)
          {
            return q1+q2;
          }).release();
    };
  }
}

namespace expected_based
{
  template <class T>
  expected<T> operator+(expected<T> i, expected<T> j)
  {
    EXPECT(i_, i);
    EXPECT(j_, j);
    return i_ + j_;
  }

  expected<int> operator-(expected<int> i, expected<int> j)
  {
    return  mbind(i, [j](int i)
      {
        return mbind(j, [i](int j)
            {
              return i-j;
            });
      });
  }

  expected<int> cex_f2(int i, int j, int k)
  {
    return safe_divide(i, k) + safe_divide(j, k);
  }
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

namespace expected_based
{
  expected<int> divide0(int i, int j)
  {
    return  catch_error(safe_divide(i,j),
      [](std::exception_ptr ex) -> expected<int>
      {
        try
        {
          std::rethrow_exception(ex);
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

  expected<int> divide(int i, int j)
  {
    return safe_divide(i,j).
    recover([](std::exception_ptr ex) -> expected<int>
        {
          try
          {
            std::rethrow_exception(ex);
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

  expected<int> divide2(int i, int j)
  {
    return safe_divide(i,j).
    catch_exception<NotDivisible>([](NotDivisible& e) -> expected<int>
        {
          return e.i / e.j;
        });
  }

  expected<int> divide3(int i, int j)
  {
    return safe_divide(i,j).
    catch_exception<NotDivisible>([](NotDivisible& e)
        {
          return make_expected(e.i / e.j);
        });
  }
  expected<int> divide4(int i, int j)
  {
    return safe_divide(i,j).
    catch_exception<NotDivisible>([](NotDivisible& e)
        {
          return e.i / e.j;
        });
  }
}

void expected_test()
{
  using namespace expected_based;
  auto r = safe_divide(1, 0);
  auto r1 = ex_f1(1, 2, 0);
  auto r2 = ex_f2(1, 2, 0);
  auto mr2 = mex_f2(1, 2, 0);
  auto tr22 = then_f22(1, 2, 1);
  std::cout << *tr22 << std::endl;
  auto tr23 = then_f23(1, 2, 1);
  std::cout << *tr23 << std::endl;
  auto tr2 = then_f2(1, 2, 1);
  std::cout << *tr2 << std::endl;
  auto r3 = cex_f2(1, 2, 0);
  auto a0 = divide0(1, 0);
  auto a = divide(1, 0);
  auto a2 = divide2(1, 0);
  auto a3 = divide3(1, 0);
  auto a4 = divide4(1, 0);
}

void optional_test()
{
  using namespace optional_based;
  auto r = safe_divide(1, 0);
  auto r1 = ex_f1(1, 2, 0);
  auto r2 = ex_f2(1, 2, 0);
  auto mr2 = mex_f2(1, 2, 0);
  auto tr22 = then_f22(1, 2, 1);
  std::cout << *tr22 << std::endl;
  auto tr23 = then_f23(1, 2, 1);
  std::cout << *tr23 << std::endl;
  //auto tr2 = then_f2(1, 2, 1);
  //std::cout << *tr2 << std::endl;
  //auto r3 = cex_f2(1, 2, 0);
  //auto a0 = divide0(1, 0);
  //auto a = divide(1, 0);
  //auto a2 = divide2(1, 0);

}

int main()
{
  expected_test();
  optional_test();
  return 0;
}
