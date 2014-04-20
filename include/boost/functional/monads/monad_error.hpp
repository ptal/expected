// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_MONAD_ERROR_HPP
#define BOOST_FUNCTIONAL_MONAD_ERROR_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/meta.hpp>
#include <boost/functional/monads/monad.hpp>
#include <boost/expected/unexpected.hpp>
#include <utility>
#include <type_traits>

namespace boost
{
namespace functional
{
namespace monad_error
{
  using namespace ::boost::functional::monad;

  template <class M>
  struct monad_error_category {
    typedef M type;
  };

  template <class M>
  using monad_error_category_t = typename monad_error_category<M>::type;

  template <class Mo>
  struct monad_error_traits : std::true_type {

    template <class M>
    static constexpr auto value(M&& m) -> decltype(m.value()) { return m.value(); };

    template <class M, class E>
    static auto make_error(E&& e) -> decltype(make_unexpected(std::forward<E>(e)))
    {
      return make_unexpected(std::forward<E>(e));
    }

    template <class M, class F>
    static M catch_error(M&& m, F&& f)
    {
      return m.catch_error(std::forward<F>(f));
    }
  };

  template <class M, class T>
  using monad_error_traits_t = monad_error_traits<monad_error_category_t<decay_t<apply<M, T> > > >;

  template <class M>
  using monad_error_traits_t0 = monad_error_traits<monad_error_category_t<decay_t<M> > > ;

  template <class M, class E, class Traits = monad_error_traits_t0<M> >
  auto make_error(E&& e) -> decltype(Traits::template make_error<M>(std::forward<E>(e)))
  {
    return Traits::template make_error<M>(std::forward<E>(e));
  }

  template <template <class ...> class M, class E, class Traits = monad_error_traits_t0<lift<M>> >
  auto make_error(E&& e) -> decltype(Traits::template make_error<M>(std::forward<E>(e)))
  {
    return Traits::template make_error<M>(std::forward<E>(e));
  }

//    template <template <class ...> class M, class T, class E, class Traits = monad_error_traits<monad_error_category_t<M<T> > > >
//    M<T, E> make_error(E&& e)
//    {
//      return Traits::template make_error<M<T,E> >(std::forward<T>(e));
//    }

  template <class M, class Traits = monad_error_traits_t0<M> >
  static constexpr auto
  value(M&& e) -> decltype(Traits::value(std::forward<M>(e)))
  {
    return Traits::value(std::forward<M>(e));
  }

  template <class M, class F, class Traits = monad_error_traits_t0<M> >
  static M catch_error(M&& m, F&& f)
  {
    return Traits::catch_error(std::forward<M>(m), std::forward<F>(f));
  }

}
}
}

template <class M, class F>
auto operator|(M&& m, F&& f)
-> decltype(::boost::functional::monad_error::catch_error(std::forward<M>(m), std::forward<F>(f)))
{
  return ::boost::functional::monad_error::catch_error(std::forward<M>(m), std::forward<F>(f));
}
#endif // BOOST_FUNCTIONAL_MONAD_ERROR_HPP
