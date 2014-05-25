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

namespace boost
{
namespace functional
{
  template <class V, class E, class T>
  struct rebindable_traits<expected<E, V, T> > : rebindable_traits<category::forward> {};

  template <class E, class V, class T>
  struct valued_traits<expected<E, V, T>> : valued_traits<category::pointer_like>
  {
    template <class M>
    static constexpr auto get_value(M&& m) -> decltype(m.value())
    { return m.value(); }
  };

  template <class V, class E, class T>
  struct errored_traits<expected<E, V, T> > : errored_traits<category::forward> {
    template <class M>
    static constexpr auto get_errored(M&& m) -> decltype(m.get_unexpected())
    { return m.get_unexpected();};

  };

  template <class V, class E, class T>
  struct functor_traits<expected<E, V, T> > : functor_traits<category::errored> {};

  template <class V, class E, class T>
  struct monad_traits<expected<E,V,T> > : monad_traits<category::forward> { };

  template <class V1, class E1, class T1>
  struct monad_error_traits<expected<E1,V1,T1> > : monad_error_traits<category::forward>
  {
    template <class M, class E>
    static auto make_error(E&& e) -> decltype(make_unexpected(std::forward<E>(e)))
    {
      return make_unexpected(std::forward<E>(e));
    }
  };

  template <class V1, class E1, class T1>
  struct monad_exception_traits<expected<E1,V1,T1> > : monad_exception_traits<category::forward>
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
