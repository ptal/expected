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
using namespace boost::functional;

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
  expected<std::exception_ptr, int> safe_divide(int i, int j)
  {
    using namespace boost::functional::monad_error;

    if (j == 0)
      return make_error<expected<>>(DivideByZero());
    else
      return make<expected<>>(i / j);
  }
}
namespace optional_based
{
  optional<int> safe_divide(int i, int j)
  {
    using namespace boost::functional::monad_error;
    if (j == 0)
      return make_error<optional_monad >(DivideByZero());
    else
      return make<optional_monad>(i / j);
  }

}
namespace generic_based
{
  template <class M>
  apply<M, int> safe_divide(int i, int j)
  {
    using namespace boost::functional::monad_error;

    if (j == 0)
      return make_error<M>(DivideByZero());
    else
      return make<M>(i / j);
  }
}
#ifdef expect
expected<std::exception_ptr, int> ex_f1(int i, int j, int k)
{
  return i + expect safe_divide(j,k);
}
expected<std::exception_ptr, int> ex_f1(int i, int j, int k)
{
  auto q = expect safe_divide(j,k);
  return i + q;
}

expected<std::exception_ptr, int> ex_f2(int i, int j, int k)
{
  return expect safe_divide(i,k) + expect safe_divide(j,k);
}

expected<std::exception_ptr, int> ex_f2(int i, int j, int k)
{
  auto s1 = expect safe_divide(i,k);
  auto s2 = expect safe_divide(j,k);
  return s1 + s2;
}
#endif

namespace expected_based
{
  using namespace boost::functional::errored;

  expected<std::exception_ptr, int> ex_f1(int i, int j, int k)
  {
    auto eq = safe_divide(j, k);
    if (! has_value(eq)) return get_unexpected(eq);
    auto q = deref(eq);

    return 1 + q;
  }

  expected<std::exception_ptr, int> ex_f2(int i, int j, int k)
  {
    auto eq1 = safe_divide(i, k);
    if (! has_value(eq1)) return get_unexpected(eq1);
    auto q1 = deref(eq1);

    auto eq2 = safe_divide(j, k);
    if (! has_value(eq2)) return get_unexpected(eq2);
    auto q2 = deref(eq2);

    return q1 + q2;
  }
}
namespace optional_based
{
  using namespace boost::functional::errored;

  optional<int> ex_f1(int i, int j, int k)
  {
    auto eq = safe_divide(j, k);
    if (! has_value(eq)) return get_unexpected(eq);
    auto q = deref(eq);

    return 1 + q;
  }

  optional<int> ex_f2(int i, int j, int k)
  {
    auto eq1 = safe_divide(i, k);
    if (!eq1) return get_unexpected(eq1);
    auto q1 = deref(eq1);

    auto eq2 = safe_divide(j, k);
    if (!eq2) return get_unexpected(eq2);
    auto q2 = deref(eq2);

    return q1 + q2;
  }
}
namespace generic_based
{
  using namespace boost::functional::errored;

  template <class M>
  apply<M, int> ex_f1(int i, int j, int k)
  {
    auto eq = safe_divide<M>(j, k);
    if (! has_value(eq)) return get_unexpected(eq);
    auto q = deref(eq);

    return 1 + q;
  }

  template <class M>
  apply<M, int> ex_f2(int i, int j, int k)
  {
    auto eq1 = safe_divide<M>(i, k);
    if (! has_value(eq1)) return get_unexpected(eq1);
    auto q1 = deref(eq1);

    auto eq2 = safe_divide<M>(j, k);
    if (! has_value(eq2)) return get_unexpected(eq2);
    auto q2 = deref(eq2);

    return q1 + q2;
  }
}

#define EXPECT(V, EXPR) \
auto BOOST_JOIN(expected,V) = EXPR; \
if (! boost::functional::valued::has_value(BOOST_JOIN(expected,V))) return boost::functional::errored::get_unexpected(BOOST_JOIN(expected,V)); \
auto V = boost::functional::valued::deref(BOOST_JOIN(expected,V))

namespace expected_based
{
  expected<std::exception_ptr, int> mex_f2(int i, int j, int k)
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
namespace generic_based
{
  template <class M>
  apply<M, int> mex_f2(int i, int j, int k)
  {
    EXPECT(q1, safe_divide<M>(i,k));
    EXPECT(q2, safe_divide<M>(j,k));
    return q1 + q2;
  }
}
namespace expected_based
{
  expected<std::exception_ptr, int> then_f22(int i, int j, int k)
  {
    using namespace boost::functional::monad_error;
    return mbind(safe_divide(i, k),
      [=](int q1)
      {
        using namespace boost::functional::monad_error;
        return mbind(safe_divide(j,k), [=](int q2)
            {
              return q1+q2;
            });
      });
  }

  expected<std::exception_ptr, int> then_f23(int i, int j, int k)
  {
    using namespace boost::functional::functor;
    return fmap([=](int q1, int q2)
        { return q1+q2;},
        safe_divide(i, k), safe_divide(j, k));
  }

  expected<std::exception_ptr, int> then_f2(int i, int j, int k)
  {
    return safe_divide(i, k) & [=](int q1)
    {
      return safe_divide(j,k) & [=](int q2)
          {
            return q1+q2;
          };
    };
  }
}

namespace optional_based
{
  optional<int> then_f22(int i, int j, int k)
  {
    using namespace boost::functional::monad_error;
    return  mbind(safe_divide(i, k),
      [=](int q1)
      {
        using namespace boost::functional::monad_error;
        return mbind(safe_divide(j,k), [=](int q2)
            {
              return q1+q2;
            });
      });
  }

  optional<int> then_f23(int i, int j, int k)
  {
    using namespace boost::functional::monad_error;
    return fmap([=](int q1, int q2)
        { return q1+q2;},
        safe_divide(i, k), safe_divide(j, k));
  }

  optional<int> then_f2(int i, int j, int k)
  {
    using namespace boost::functional::monad_error;
    return safe_divide(i, k) & [=](int q1)
    {
      using namespace boost::functional::monad_error;
      return safe_divide(j,k) & [=](int q2)
          {
            return q1+q2;
          };
    };
  }
}

namespace generic_based
{
  template <class M>
  apply<M, int> then_f22(int i, int j, int k)
  {
    using namespace boost::functional::monad_error;
    return  mbind(safe_divide<M>(i, k),
      [=](int q1)
      {
        using namespace boost::functional::monad_error;
        return mbind(safe_divide<M>(j,k), [=](int q2)
            {
              return q1+q2;
            });
      });
  }

  template <class M>
  apply<M, int> then_f23(int i, int j, int k)
  {
    using namespace boost::functional::functor;
    return fmap([=](int q1, int q2)
        { return q1+q2;},
        safe_divide<M>(i, k), safe_divide<M>(j, k));
  }

  template <class M>
  apply<M, int> then_f2(int i, int j, int k)
  {
    return safe_divide<M>(i, k) & [=](int q1)
    {
      return safe_divide<M>(j,k) & [=](int q2)
          {
            return q1+q2;
          };
    };
  }
}

namespace expected_based
{
  using namespace boost::functional::monad_error;
  template <class T>
  expected<std::exception_ptr, T> operator+(expected<std::exception_ptr, T> i, expected<std::exception_ptr, T> j)
  {
    EXPECT(i_, i);
    EXPECT(j_, j);
    return i_ + j_;
  }

  expected<std::exception_ptr, int> operator-(expected<std::exception_ptr, int> i, expected<std::exception_ptr, int> j)
  {
    return  mbind(i, [j](int i)
      {
        return mbind(j, [i](int j)
            {
              return i-j;
            });
      });
  }

  expected<std::exception_ptr, int> cex_f2(int i, int j, int k)
  {
    return safe_divide(i, k) + safe_divide(j, k);
  }
}
namespace optional_based
{
  using namespace boost::functional::monad_error;
  template <class T>
  optional<T> operator+(optional<T> i, optional<T> j)
  {
    EXPECT(i_, i);
    EXPECT(j_, j);
    return i_ + j_;
  }

  template <class T>
  optional<T> operator-(optional<T> i, optional<T> j)
  {
    return  mbind(i, [j](T i)
      {
        return mbind(j, [i](T j)
            {
              return i-j;
            });
      });
  }

  optional<int> cex_f2(int i, int j, int k)
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
  expected<std::exception_ptr, int> divide0(int i, int j)
  {
    using namespace boost::functional::monad_error;
    return  catch_error(safe_divide(i,j),
      [](std::exception_ptr ex) -> expected<std::exception_ptr, int>
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
          return make_error<expected<>>(std::current_exception());
        }
      });
  }

//  expected<std::exception_ptr, int> divide2(int i, int j)
//  {
//    return safe_divide(i,j).
//    catch_exception<NotDivisible>([](NotDivisible& e) -> expected<std::exception_ptr, int>
//        {
//          return e.i / e.j;
//        });
//  }

//  expected<std::exception_ptr, int> divide3(int i, int j)
//  {
//    return safe_divide(i,j).
//    catch_exception<NotDivisible>([](NotDivisible& e)
//        {
//          return make_expected(e.i / e.j);
//        });
//  }
//  expected<std::exception_ptr, int> divide4(int i, int j)
//  {
//    return safe_divide(i,j).
//    catch_exception<NotDivisible>([](NotDivisible& e)
//        {
//          return e.i / e.j;
//        });
//  }
}

namespace optional_based
{
//  optional<int> divide0(int i, int j)
//  {
//    using namespace boost::functional::monad_error;
//    return  catch_error(safe_divide(i,j),
//      [](std::exception_ptr ex) -> optional<int>
//      {
//        try
//        {
//          std::rethrow_exception(ex);
//        }
//        catch(NotDivisible& e)
//        {
//          return e.i / e.j;
//        }
//        catch (...)
//        {
//          using namespace boost::functional::monad_error;
//          return make_error<optional_monad>(std::current_exception());
//        }
//      });
//  }

//  expected<std::exception_ptr, int> divide2(int i, int j)
//  {
//    return safe_divide(i,j).
//    catch_exception<NotDivisible>([](NotDivisible& e) -> expected<std::exception_ptr, int>
//        {
//          return e.i / e.j;
//        });
//  }

//  expected<std::exception_ptr, int> divide3(int i, int j)
//  {
//    return safe_divide(i,j).
//    catch_exception<NotDivisible>([](NotDivisible& e)
//        {
//          return make_expected(e.i / e.j);
//        });
//  }
//  expected<std::exception_ptr, int> divide4(int i, int j)
//  {
//    return safe_divide(i,j).
//    catch_exception<NotDivisible>([](NotDivisible& e)
//        {
//          return e.i / e.j;
//        });
//  }
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
  //auto a2 = divide2(1, 0);
  //auto a3 = divide3(1, 0);
  //auto a4 = divide4(1, 0);
}

template <class M>
void generic_test()
{
  using namespace generic_based;
  auto r = safe_divide<M>(1, 0);
  auto r1 = ex_f1<M>(1, 2, 0);
  auto r2 = ex_f2<M>(1, 2, 0);
  auto mr2 = mex_f2<M>(1, 2, 0);
  auto tr22 = then_f22<M>(1, 2, 1);
  std::cout << *tr22 << std::endl;
  auto tr23 = then_f23<M>(1, 2, 1);
  std::cout << *tr23 << std::endl;
  auto tr2 = then_f2<M>(1, 2, 1);
  std::cout << *tr2 << std::endl;

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
  auto tr2 = then_f2(1, 2, 1);
  std::cout << *tr2 << std::endl;
  auto r3 = cex_f2(1, 2, 0);
  //auto a0 = divide0(1, 0);
  //auto a = divide(1, 0);
  //auto a2 = divide2(1, 0);

}

int main()
{
  expected_test();
  optional_test();
  generic_test<expected<>>();
  generic_test<optional_monad>();
  return 0;
}
