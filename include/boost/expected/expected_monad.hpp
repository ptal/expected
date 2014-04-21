// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_MONAD_HPP

#include <boost/functional/monads/categories/errored.hpp>
#include <boost/functional/monads/monad_error.hpp>
#include <boost/expected/expected.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/mpl/identity.hpp>
#include <type_traits>

namespace boost
{
namespace functional
{
  template <class T, class E>
  struct rebindable_category<expected<E, T> > : mpl::identity<category::forward> {};

  template <class E, class T>
  struct valued_traits<expected<E, T>> : std::true_type
  {
    template <class M>
    static constexpr bool has_value(M&& m) { return bool(m); }

    template <class M>
    static constexpr auto deref(M&& m) -> decltype(*m) { return *m; }

    template <class M>
    static constexpr rebindable::value_type<M> get_value(M&& m) { return m.value(); };
  };

  template <class T, class E>
  struct unexpected_category<expected<E, T> > : mpl::identity<category::forward> {};

  template <class T, class E>
  struct functor_category<expected<E, T> > : mpl::identity<category::errored> {};

  template <class T, class E>
  struct monad_category<expected<E,T> > : mpl::identity<category::forward> { };

  template <class T1, class E1>
  struct monad_error_traits<expected<E1,T1> > : std::true_type
  {
    template <class M>
    static constexpr auto value(M&& m) -> decltype(m.value())
    { return m.value();};

    template <class M, class E>
    static auto make_error(E&& e) -> decltype(make_unexpected(std::forward<E>(e)))
    {
      return make_unexpected(std::forward<E>(e));
    }

    template <class M, class F>
    static M catch_error(M&& m, F&& f)
    {
      return m.recover(std::forward<F>(f));
    }
  };
}
}

#endif // BOOST_EXPECTED_EXPECTED_MONAD_HPP
