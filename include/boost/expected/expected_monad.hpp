// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_MONAD_HPP

#include <boost/functional/monads/categories/errored.hpp>
#include <boost/functional/monads/categories/pointer_like.hpp>
#include <boost/functional/monads/monad_exception.hpp>
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
  struct valued_traits<expected<E, T>> : valued_traits<category::pointer_like>
  {
    template <class M>
    static constexpr rebindable::value_type<M> get_value(M&& m) { return m.value(); };
  };

  template <class T, class E>
  struct errored_category<expected<E, T> > : mpl::identity<category::forward> {};

  template <class T, class E>
  struct functor_category<expected<E, T> > : mpl::identity<category::errored> {};

  template <class T, class E>
  struct monad_category<expected<E,T> > : mpl::identity<category::forward> { };

  template <class T1, class E1>
  struct monad_error_traits<expected<E1,T1> > : monad_error_traits<category::forward>
  {
    template <class M, class E>
    static auto make_error(E&& e) -> decltype(make_unexpected(std::forward<E>(e)))
    {
      return make_unexpected(std::forward<E>(e));
    }
  };

  template <class T1, class E1>
  struct monad_exception_traits<expected<E1,T1> > : monad_exception_traits<category::forward>
  {
    template <class M, class E>
    static auto make_exception(E&& e) -> decltype(make_unexpected(std::forward<E>(e)))
    {
      return make_unexpected(std::forward<E>(e));
    }
  };
}
}

#endif // BOOST_EXPECTED_EXPECTED_MONAD_HPP
