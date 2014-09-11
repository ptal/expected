// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_MONADS_CATEGORIES_VALUED_AND_ERRORED_HPP
#define BOOST_EXPECTED_MONADS_CATEGORIES_VALUED_AND_ERRORED_HPP

#include <boost/config.hpp>
#include <boost/functional/monads/errored.hpp>
#include <boost/functional/monads/functor.hpp>
#include <boost/functional/monads/monad.hpp>
#include <boost/functional/monads/algorithms/have_value.hpp>
#include <boost/functional/monads/algorithms/first_unexpected.hpp>
#include <boost/functional/meta.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <type_traits>

#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
#if defined __clang__
#if ! __has_feature(cxx_relaxed_constexpr)
#define BOOST_NO_CXX14_RELAXED_CONSTEXPR
#endif
#else
#define BOOST_NO_CXX14_RELAXED_CONSTEXPR
#endif
#endif

#define REQUIRES(...) typename ::boost::enable_if_c<__VA_ARGS__, void*>::type = 0
#define T_REQUIRES(...) typename = typename ::boost::enable_if_c<(__VA_ARGS__)>::type

namespace boost
{
namespace functional
{
  namespace category
  {
    struct errored {};
  }

  template <>
  struct functor_traits<category::errored> : functor_traits<category::default_>
  {

    template <class F, class M0, class ...M,
    class FR = decltype( std::declval<F>()(errored::deref(std::declval<M0>()), errored::deref(std::declval<M>())...) )>
    static BOOST_EXPECTED_RELAXED_CONSTEXPR auto map(F&& f, M0&& m0, M&& ...m) -> errored::rebind<decay_t<M0>, FR>
    {
      using namespace errored;
      typedef rebind<decay_t<M0>, FR> result_type;
      return have_value( std::forward<M0>(m0), std::forward<M>(m)... )
      ? result_type( std::forward<F>(f)( deref(std::forward<M0>(m0)), deref(std::forward<M>(m))... ) )
      : first_unexpected( std::forward<M0>(m0), std::forward<M>(m)... )
      ;
    }
  };

  template <>
  struct monad_traits<category::errored> : monad_traits<category::forward>
  {
    template <class M, class F, class FR = decltype( std::declval<F>()( errored::deref(std::declval<M>()) ) )>
    static BOOST_EXPECTED_RELAXED_CONSTEXPR auto
    bind(M&& m, F&& f,
        REQUIRES(boost::is_same<FR, void>::value)
    ) -> errored::rebind<decay_t<M>, FR>
    {
      using namespace errored;
      typedef rebind<decay_t<M>, FR> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
      if(has_value(m))
      {
        f(deref(m));
        return result_type();
      }
      return get_errored(m);
#else
      return (has_value(m)
          ? (f(deref(m)), result_type() )
          : result_type( get_errored(m) )
      );
#endif
    }

    template <class M, class F, class FR = decltype( std::declval<F>()( errored::deref(std::declval<M>()) ) )>
    static BOOST_EXPECTED_RELAXED_CONSTEXPR auto
    bind(M&& m, F&& f,
        REQUIRES((! boost::is_same<FR, void>::value
                && ! boost::functional::is_monad<FR>::value)
        )) -> errored::rebind<decay_t<M>, FR>
    {
      using namespace errored;
      typedef rebind<decay_t<M>, FR> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
      if(has_value(m))
      {
        return result_type(f(deref(m)));
      }
      return get_errored(m);
#else
      return (has_value(m)
          ? result_type(f(deref(m)))
          : result_type( get_errored(m) )
      );
#endif
    }

    template <class M, class F, class FR = decltype( std::declval<F>()( errored::deref(std::declval<M>()) ) )>
    static BOOST_EXPECTED_RELAXED_CONSTEXPR auto
    bind(M&& m, F&& f,
        REQUIRES( boost::functional::is_monad<FR>::value )
    ) -> FR
    {
      using namespace errored;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
      if(has_value(m))
      {
        return f(deref(m));
      }
      return get_errored(m);
#else
      return (has_value(m)
          ? f(deref(m))
          : FR( get_errored(m) )
      );
#endif
    }
  };

}
}

#undef REQUIRES
#undef T_REQUIRES
#endif // BOOST_EXPECTED_MONADS_CATEGORIES_VALUED_AND_ERRORED_HPP
