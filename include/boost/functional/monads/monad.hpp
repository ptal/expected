// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_MONAD_HPP
#define BOOST_FUNCTIONAL_MONAD_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/meta.hpp>
#include <boost/functional/monads/functor.hpp>
#include <utility>

namespace boost
{
namespace functional
{
namespace monad
{
  using namespace ::boost::functional::functor;

  template <class M>
  struct is_monad: std::false_type {};

  template <class M>
  struct monad_category
  {
    typedef M type;
  };

  template <class M>
  using monad_category_t = typename monad_category<M>::type;

  template <class Mo>
  struct monad_traits
  {
    // make use of constructor
    template <class M, class T>
    static apply<M, T> make(T&& v)
    {
      return apply<M, T>(std::forward<T>(v));
    }

    // make use of member function
    template <class M, class F>
    static auto
    mbind(M&& m, F&& f) -> decltype(m.mbind(std::forward<F>(f)))
    {
      return m.mbind(std::forward<F>(f));
    }

  };

  template <class M, class T>
  using monad_traits_t = monad_traits<monad_category_t<decay_t<apply<M, T> > > >;

  template <class M>
  using monad_traits_t0 = monad_traits<monad_category_t<decay_t<M> > >;

  template <class M, class T, class Traits = monad_traits_t<M,T> >
  apply<M,T> make(T&& v)
  {
    return Traits::template make<M>(std::forward<T>(v));
  }

  template <template <class ...> class M, class T, class Traits = monad_traits_t<lift<M>,T> >
  M<T> make(T&& v)
  {
    return Traits::template make<lift<M>>(std::forward<T>(v));
  }

  template <class M, class F, class Traits = monad_traits_t0<M> >
  auto
  mbind(M&& m, F&& f) -> decltype(Traits::mbind(std::forward<M>(m), std::forward<F>(f)))
  {
    return Traits::mbind(std::forward<M>(m), std::forward<F>(f));
  }
}
}
}

template <class M, class F>
auto operator&(M&& m, F&& f)
-> decltype(::boost::functional::monad::mbind(std::forward<M>(m), std::forward<F>(f)))
{
  return ::boost::functional::monad::mbind(std::forward<M>(m),std::forward<F>(f));
}

#endif // BOOST_FUNCTIONAL_MONAD_HPP
