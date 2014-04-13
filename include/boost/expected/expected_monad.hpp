// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_MONAD_HPP

#include <boost/functional/monads/categories/pointer_like.hpp>
#include <boost/functional/monads/categories/valued_and_errored.hpp>
#include <boost/functional/monads/monad_error.hpp>
#include <boost/expected/expected.hpp>
#include <boost/mpl/identity.hpp>
#include <type_traits>

namespace boost
{
  namespace monads
  {

    template <class T, class E>
    struct is_monad<expected<E,T> > : std::true_type { };

    template <class T, class E>
    struct value_category<expected<E,T> > : mpl::identity<category::pointer_like> { };
    template <class T, class E>
    struct functor_category<expected<E,T> > : mpl::identity<category::valued_and_errored> { };
    template <class T, class E>
    struct monad_category<expected<E,T> > : mpl::identity<category::valued_and_errored> { };

    template <class T1, class E1>
    struct monad_error_traits<expected<E1,T1> >
    {
      template <class M>
      static constexpr auto value(M&& m) -> decltype(m.value()) { return m.value(); };

      template <class M, class E>
      static M make_error(E&& v)
      {
        return make_unexpected(std::forward<E>(v));
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

#endif // BOOST_EXPECTED_EXPECTED_MONAD_HPP
