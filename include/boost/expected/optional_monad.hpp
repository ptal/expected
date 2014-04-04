// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_OPTIONAL_MONAD_HPP
#define BOOST_EXPECTED_OPTIONAL_MONAD_HPP

#include <boost/expected/expected_like_monad.hpp>
#include <boost/optional.hpp>
#include <boost/mpl/identity.hpp>

namespace boost
{
  namespace monads
  {

    template <class T>
    struct is_monad<optional<T> > : std::true_type {};
    template <class T, class U>
    struct bind<optional<T>, U> : mpl::identity<optional<U> > {};
    template <class T>
    struct functor_category<optional<T> > : mpl::identity<category::expected_like> { };
    template <class T>
    struct monad_category<optional<T> > : mpl::identity<category::expected_like> { };
    template <class T>
    struct value_category<optional<T> > : mpl::identity<category::pointer_like> { };

    template <class T>
    struct unexpected_traits< optional<T> > {
      template< class M >
      using type = none_t;

      template <class M, class E>
      static M make_error(E&&)
      {
        return none;
      }
      static constexpr none_t get_unexpected(optional<T> const& ) { return none; }
      template< class M >
      static constexpr none_t error(M && m) { return none; }

    };

    template <class T>
    struct monad_error_traits<optional<T> > : monad_traits<monad_category<optional<T>>>
    {
      template <class M>
      static constexpr auto value(M&& m) -> decltype(m.value()) { return m.value(); };

      template <class M, class E>
      static M make_error(E&&)
      {
        return none;
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


#endif // BOOST_EXPECTED_OPTIONAL_MONAD_HPP
