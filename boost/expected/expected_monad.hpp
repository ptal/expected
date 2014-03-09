// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_MONAD_HPP
#define BOOST_EXPECTED_MONAD_HPP

#include <boost/expected/expected.hpp>
#include <boost/functional/monad.hpp>

namespace boost
{
  namespace monads
  {

    template <class X, class E>
    struct functor_traits<expected<X, E>>
    {
      template< class M >
      static constexpr bool each( const M& m )
      {
        return (bool)m;
      }

      template< class M1, class ...Ms >
      static constexpr bool each( const M1& m1, const Ms& ...ms )
      {
        return (bool)m1 && each( ms... );
      }

      template< class M >
      static constexpr E error( const M& m )
      {
        return m.error();
      }

      template< class M1, class ...Ms >
      static constexpr E error( const M1& m1, const Ms& ...ms )
      {
        return (bool)m1 ? error( ms... ) : m1.error();
      }

      template <class F, class M0, class ...M,
          class FR = decltype( std::declval<F>()(*std::declval<M0>(), *std::declval<M>()...) )>
      static auto when_all_valued(F&& f, M0&& m0, M&& ...m) -> typename bind<decay_t<M0>, FR>::type
      {
        typedef typename bind<decay_t<M0>, FR>::type expected_type;
        return each( std::forward<M0>(m0), std::forward<M>(m)... )
        ? expected_type( std::forward<F>(f)( *std::forward<M0>(m0), *std::forward<M>(m)... ) )
        : make_unexpected(error( std::forward<M0>(m0), std::forward<M>(m)... ))
        ;
      }
    };

    template <class X, class E>
    struct monad_traits<expected<X,E>>
    {

      template <class M, class T>
      static M make(T&& v)
      {
        return M(std::forward<T>(v));
      }

      template <class M, class F>
      static auto
      when_ready(M&& m, F&& f) -> decltype(m.then(std::forward<F>(f)))
      {
        m.then(std::forward<F>(f));
      }

      template <class M, class F>
      static auto
      when_valued(M&& m, F&& f) -> decltype(m.next(std::forward<F>(f)))
      {
        return m.next(std::forward<F>(f));
      }

    };

    template <class X, class E>
    struct monad_error_traits<expected<X,E>> : monad_traits<expected<X,E>>
    {
      template <class M, class F>
      static auto
      when_unexpected(M&& m, F&& f) -> decltype(m.recover(std::forward<F>(f)))
      {
        return m.recover(std::forward<F>(f));
      }
    };
  }
}

#endif // BOOST_EXPECTED_MONAD_HPP
