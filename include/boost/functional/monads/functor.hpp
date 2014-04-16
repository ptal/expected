// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_FUNCTOR_HPP
#define BOOST_FUNCTIONAL_FUNCTOR_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/meta.hpp>
#include <utility>

namespace boost
{
namespace functional
{
namespace functor
{
  template <class M>
  struct functor_category
  {
    typedef M type;
  };

  template <class M>
  using functor_category_t = typename functor_category<M>::type;

  template <class Mo>
  struct functor_traits
  {

    template <class F, class M0, class ...M, class FR = decltype( std::declval<F>()(*std::declval<M0>(), *std::declval<M>()...) )>
    static auto
    fmap(F&& f, M0&& m0, M&& ...ms) -> typename functional::rebind<decay_t<M0>, FR>::type
    {
      return M0::fmap(std::forward<F>(f), std::forward<M0>(m0), std::forward<M>(ms)...);
    }
  };

  template <class F, class M0, class ...M, class Traits = functor_traits<functor_category_t<decay_t<M0> > > >
  auto
  fmap(F&& f, M0&& m0, M&& ...m)
  -> decltype(Traits::fmap(std::forward<F>(f), std::forward<M0>(m0), std::forward<M>(m)...))
  {
    return Traits::fmap(std::forward<F>(f),std::forward<M0>(m0), std::forward<M>(m)...);
  }
}
}
}

#endif // BOOST_FUNCTIONAL_FUNCTOR_HPP
