// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_FUNCTOR_HPP
#define BOOST_FUNCTIONAL_FUNCTOR_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/meta.hpp>
#include <boost/functional/monads/rebindable.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <utility>
#include <type_traits>

namespace boost
{
namespace functional
{
  template <class M>
  struct functor_category
  {
    typedef M type;
  };

  template <class M>
  using functor_category_t = typename functor_category<M>::type;

  template <class Mo>
  struct functor_traits : std::false_type {};
  template <>
  struct functor_traits<category::forward> : std::true_type
  {

    template <class F, class M0, class ...M, class FR = decltype( std::declval<F>()(*std::declval<M0>(), *std::declval<M>()...) )>
    static auto
    fmap(F&& f, M0&& m0, M&& ...ms) -> rebindable::rebind<decay_t<M0>, FR>
    {
      return M0::fmap(std::forward<F>(f), std::forward<M0>(m0), std::forward<M>(ms)...);
    }
  };

namespace functor
{
  using namespace ::boost::functional::rebindable;

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
