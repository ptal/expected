// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_MONAD_EXCEPTION_HPP
#define BOOST_FUNCTIONAL_MONAD_EXCEPTION_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/meta.hpp>
#include <boost/functional/monads/monad.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <boost/functional/monads/monad_error.hpp>
#include <utility>
#include <type_traits>

namespace boost
{
namespace functional
{
  template <class M>
  struct monad_exception_category {
    typedef M type;
  };

  template <class M>
  using monad_exception_category_t = typename monad_exception_category<M>::type;

  template <class Mo>
  struct monad_exception_traits : std::false_type {};
  template <>
  struct monad_exception_traits<category::forward> : std::true_type
  {
    template <class M, class E>
    static auto make_exception(E&& e) -> decltype(make_exception(std::forward<E>(e)))
    {
      return make_exception(std::forward<E>(e));
    }

    template <class E, class M>
    static bool has_exception(M&& m)
    {
      return m.template has_exception<E>();
    }

    template <class E, class M, class F>
    static M catch_exception(M&& m, F&& f)
    {
      return m.template catch_exception<E>(std::forward<F>(f));
    }
  };

  template <class M, class T>
  using monad_exception_traits_t = monad_exception_traits<monad_exception_category_t<decay_t<apply<M, T> > > >;

  template <class M>
  using monad_exception_traits_t0 = monad_exception_traits<monad_exception_category_t<decay_t<M> > > ;

namespace monad_exception
{
  using namespace ::boost::functional::monad_error;

  template <class M, class E, class Traits = monad_exception_traits_t0<M> >
  auto make_exception(E&& e) -> decltype(Traits::template make_exception<M>(std::forward<E>(e)))
  {
    return Traits::template make_exception<M>(std::forward<E>(e));
  }

  template <template <class ...> class M, class E, class Traits = monad_exception_traits_t0<lift<M>> >
  auto make_exception(E&& e) -> decltype(Traits::template make_exception<M>(std::forward<E>(e)))
  {
    return Traits::template make_error<M>(std::forward<E>(e));
  }

//    template <template <class ...> class M, class T, class E, class Traits = monad_exception_traits<monad_exception_category_t<M<T> > > >
//    M<T, E> make_error(E&& e)
//    {
//      return Traits::template make_error<M<T,E> >(std::forward<T>(e));
//    }

  template <class E, class M, class Traits = monad_exception_traits_t0<M> >
  static bool has_exception(M&& m)
  {
    return Traits::template has_exception<E>(std::forward<M>(m));
  }
  template <class E, class M, class Traits = monad_exception_traits_t0<M> >
  static M catch_exception(M&& m)
  {
    return Traits::template catch_exception<E>(std::forward<M>(m));
  }

}
}
}

#endif // BOOST_FUNCTIONAL_MONAD_EXCEPTION_HPP
