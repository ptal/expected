// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP

#include <boost/functional/monad.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/optional.hpp>

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
    struct functor_traits<category::expected_like>
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
      static constexpr unexpected_type_t<M> the_unexpected( const M& m )
      {
        return get_unexpected(m);
      }

      template< class M1, class ...Ms >
      static constexpr unexpected_type_t<M1> the_unexpected( const M1& m1, const Ms& ...ms )
      {
        return (bool)m1 ? the_unexpected( ms... ) : get_unexpected(m1) ;
      }

      template <class F, class M0, class ...M,
          class FR = decltype( std::declval<F>()(*std::declval<M0>(), *std::declval<M>()...) )>
      static auto when_all_valued(F&& f, M0&& m0, M&& ...m) -> typename bind<decay_t<M0>, FR>::type
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

#ifdef FORWARD_TO_EXPECTED
      template <class M, class F>
      static auto
      when_valued(M&& m, F&& f) -> decltype(m.next(std::forward<F>(f)))
      {
        return m.next(std::forward<F>(f));
      }
#else
      template <class M, class F, class FR = decltype( std::declval<F>()( *std::declval<M>() ) )>
      static auto
      when_valued(M&& m, F&& f,
          REQUIRES(boost::is_same<FR, void>::value)
      ) -> typename bind<decay_t<M>, FR>::type
      {
        typedef typename bind<decay_t<M>, FR>::type result_type;
        if(m)
        {
          f(*m);
          return result_type();
        }
        return get_unexpected(m);
      }

      template <class M, class F, class FR = decltype( std::declval<F>()( *std::declval<M>() ) )>
      static auto
      when_valued(M&& m, F&& f,
          REQUIRES((! boost::is_same<FR, void>::value
              &&    ! boost::monads::is_monad<FR>::value)
      )) -> typename bind<decay_t<M>, FR>::type
      {
        typedef typename bind<decay_t<M>, FR>::type result_type;
        if(m)
        {
            return result_type(f(*m));
        }
        return get_unexpected(m);
      }

      template <class M, class F, class FR = decltype( std::declval<F>()( *std::declval<M>() ) )>
      static auto
      when_valued(M&& m, F&& f,
          REQUIRES( boost::monads::is_monad<FR>::value )
      ) -> FR
      {
        if(m)
        {
            return f(*m);
        }
        return get_unexpected(m);
      }
#endif
    };

    template <>
    struct monad_error_traits<category::expected_like> : monad_traits<category::expected_like>
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

#undef REQUIRES
#undef T_REQUIRES

#endif // BOOST_EXPECTED_EXPECTED_LIKE_MONAD_HPP
