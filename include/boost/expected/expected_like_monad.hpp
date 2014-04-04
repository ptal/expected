// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP

#include <boost/config.hpp>
#include <boost/functional/monads.hpp>
#include <boost/functional/monads/categories/pointer_like.hpp>
#include <boost/functional/monads/algorithms/have_value.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

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
  namespace monads
  {
    namespace category
    {
      struct expected_like {};
    }

    template< class M >
    BOOST_CONSTEXPR unexpected_type_t<M> first_unexpected( M&& m )
    {
      return get_unexpected(std::forward<M>(m));
    }
    template< class M1, class ...Ms >
    BOOST_CONSTEXPR unexpected_type_t<M1> first_unexpected( M1&& m1, Ms&& ...ms )
    {
      return has_value(std::forward<M1>(m1))
          ? first_unexpected( std::forward<Ms>(ms)... )
              : get_unexpected(std::forward<M1>(m1)) ;
    }

    template <>
    struct functor_traits<category::expected_like>
    {
      template <class F, class M0, class ...M,
          class FR = decltype( std::declval<F>()(deref(std::declval<M0>()), deref(std::declval<M>())...) )>
      static BOOST_CONSTEXPR auto fmap(F&& f, M0&& m0, M&& ...m) -> typename bind<decay_t<M0>, FR>::type
      {
        typedef typename bind<decay_t<M0>, FR>::type expected_type;
        return have_value( std::forward<M0>(m0), std::forward<M>(m)... )
        ? expected_type( std::forward<F>(f)( deref(std::forward<M0>(m0)), deref(std::forward<M>(m))... ) )
        : first_unexpected( std::forward<M0>(m0), std::forward<M>(m)... )
        ;
      }
    };

    template <>
    struct monad_traits<category::expected_like>
    {

      template <class M, class T>
      static BOOST_CONSTEXPR M make(T&& v)
      {
        return M(std::forward<T>(v));
      }

      template <class M, class F>
      static BOOST_CONSTEXPR auto
      then(M&& m, F&& f) -> decltype(m.then(std::forward<F>(f)))
      {
        return m.then(std::forward<F>(f));
      }

#ifdef FORWARD_TO_EXPECTED
      template <class M, class F>
      static BOOST_CONSTEXPR auto
      mbind(M&& m, F&& f) -> decltype(m.next(std::forward<F>(f)))
      {
        return m.next(std::forward<F>(f));
      }
#else
      template <class M, class F, class FR = decltype( std::declval<F>()( deref(std::declval<M>()) ) )>
      static BOOST_CONSTEXPR auto
      mbind(M&& m, F&& f,
          REQUIRES(boost::is_same<FR, void>::value)
      ) -> typename bind<decay_t<M>, FR>::type
      {
        typedef typename bind<decay_t<M>, FR>::type result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if(has_value(m))
        {
          f(deref(m));
          return result_type();
        }
        return get_unexpected(m);
#else
        return (has_value(m)
        ? (f(deref(m)), result_type() )
        : result_type( get_unexpected(m) )
        );
#endif
      }

      template <class M, class F, class FR = decltype( std::declval<F>()( deref(std::declval<M>()) ) )>
      static BOOST_CONSTEXPR auto
      mbind(M&& m, F&& f,
          REQUIRES((! boost::is_same<FR, void>::value
              &&    ! boost::monads::is_monad<FR>::value)
      )) -> typename bind<decay_t<M>, FR>::type
      {
        typedef typename bind<decay_t<M>, FR>::type result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if(has_value(m))
        {
            return result_type(f(deref(m)));
        }
        return get_unexpected(m);
#else
        return (has_value(m)
        ? result_type(f(deref(m)))
        : result_type( get_unexpected(m) )
        );
#endif
      }

      template <class M, class F, class FR = decltype( std::declval<F>()( deref(std::declval<M>()) ) )>
      static BOOST_CONSTEXPR auto
      mbind(M&& m, F&& f,
          REQUIRES( boost::monads::is_monad<FR>::value )
      ) -> FR
      {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if(has_value(m))
        {
            return f(deref(m));
        }
        return get_unexpected(m);
#else
        return (has_value(m)
        ? f(deref(m))
        : FR( get_unexpected(m) )
        );
#endif
      }
#endif
    };

    template <>
    struct monad_error_traits<category::expected_like> : monad_traits<category::expected_like>
    {
      template <class M>
      static constexpr auto value(M&& m) -> decltype(m.value()) { return m.value(); };

      template <class M, class E>
      static M make_error(E&& v)
      {
        return M(make_unexpected(std::forward<E>(v)));
      }

      template <class M, class F>
      static BOOST_CONSTEXPR auto
      catch_error(M&& m, F&& f) -> decltype(m.recover(std::forward<F>(f)))
      {
        return m.recover(std::forward<F>(f));
      }
    };
  }
}

#undef REQUIRES
#undef T_REQUIRES

#endif // BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP
