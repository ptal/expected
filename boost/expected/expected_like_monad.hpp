// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP

#include <boost/functional/monad.hpp>
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

    template <>
    struct value_traits<category::expected_like> {
      template <class M>
      using type = typename M::value_type;
      template <class M>
      static constexpr bool has_value(M&& m) { return bool(m); };
      template <class M>
      static constexpr auto value(M& m) -> decltype(m.value()) { return m.value(); };
      template <class M>
      static constexpr auto value_pre_has_value(M& m) -> decltype(*m) { return *m; };
    };

    template <>
    struct unexpected_traits<category::expected_like > {
      template< class M >
      using type = unexpected_type<typename M::error_type>;
      template< class M >
      static constexpr auto get_unexpected(M && m) -> decltype(m.get_unexpected()) { return m.get_unexpected(); }
      template< class M >
      static constexpr auto error(M && m) -> decltype(m.error()) { return m.error(); }
    };

    template <>
    struct functor_traits<category::expected_like>
    {
      template< class M >
      static BOOST_CONSTEXPR bool each( const M& m )
      {
        return (bool)m;
      }

      template< class M1, class ...Ms >
      static BOOST_CONSTEXPR bool each( const M1& m1, const Ms& ...ms )
      {
        return (bool)m1 && each( ms... );
      }

      template< class M >
      static BOOST_CONSTEXPR unexpected_type_t<M> the_unexpected( const M& m )
      {
        return get_unexpected(m);
      }

      template< class M1, class ...Ms >
      static BOOST_CONSTEXPR unexpected_type_t<M1> the_unexpected( const M1& m1, const Ms& ...ms )
      {
        return (bool)m1 ? the_unexpected( ms... ) : get_unexpected(m1) ;
      }

      template <class F, class M0, class ...M,
          class FR = decltype( std::declval<F>()(*std::declval<M0>(), *std::declval<M>()...) )>
      static BOOST_CONSTEXPR auto when_all_valued(F&& f, M0&& m0, M&& ...m) -> typename bind<decay_t<M0>, FR>::type
      {
        typedef typename bind<decay_t<M0>, FR>::type expected_type;
        return each( std::forward<M0>(m0), std::forward<M>(m)... )
        ? expected_type( std::forward<F>(f)( *std::forward<M0>(m0), *std::forward<M>(m)... ) )
        : the_unexpected( std::forward<M0>(m0), std::forward<M>(m)... )
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
      when_ready(M&& m, F&& f) -> decltype(m.then(std::forward<F>(f)))
      {
        return m.then(std::forward<F>(f));
      }

#ifdef FORWARD_TO_EXPECTED
      template <class M, class F>
      static BOOST_CONSTEXPR auto
      when_valued(M&& m, F&& f) -> decltype(m.next(std::forward<F>(f)))
      {
        return m.next(std::forward<F>(f));
      }
#else
      template <class M, class F, class FR = decltype( std::declval<F>()( *std::declval<M>() ) )>
      static BOOST_CONSTEXPR auto
      when_valued(M&& m, F&& f,
          REQUIRES(boost::is_same<FR, void>::value)
      ) -> typename bind<decay_t<M>, FR>::type
      {
        typedef typename bind<decay_t<M>, FR>::type result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if(m)
        {
          f(*m);
          return result_type();
        }
        return get_unexpected(m);
#else
        return (m
        ? (f(*m), result_type() )
        : result_type( get_unexpected(m) )
        );
#endif
      }

      template <class M, class F, class FR = decltype( std::declval<F>()( *std::declval<M>() ) )>
      static BOOST_CONSTEXPR auto
      when_valued(M&& m, F&& f,
          REQUIRES((! boost::is_same<FR, void>::value
              &&    ! boost::monads::is_monad<FR>::value)
      )) -> typename bind<decay_t<M>, FR>::type
      {
        typedef typename bind<decay_t<M>, FR>::type result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if(m)
        {
            return result_type(f(*m));
        }
        return get_unexpected(m);
#else
        return (m
        ? result_type(f(*m))
        : result_type( get_unexpected(m) )
        );
#endif
      }

      template <class M, class F, class FR = decltype( std::declval<F>()( *std::declval<M>() ) )>
      static BOOST_CONSTEXPR auto
      when_valued(M&& m, F&& f,
          REQUIRES( boost::monads::is_monad<FR>::value )
      ) -> FR
      {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if(m)
        {
            return f(*m);
        }
        return get_unexpected(m);
#else
        return (m
        ? f(*m)
        : FR( get_unexpected(m) )
        );
#endif
      }
#endif
    };

    template <>
    struct monad_error_traits<category::expected_like> : monad_traits<category::expected_like>
    {
      template <class M, class F>
      static BOOST_CONSTEXPR auto
      when_unexpected(M&& m, F&& f) -> decltype(m.recover(std::forward<F>(f)))
      {
        return m.recover(std::forward<F>(f));
      }
    };
  }
}

#undef REQUIRES
#undef T_REQUIRES

#endif // BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP
