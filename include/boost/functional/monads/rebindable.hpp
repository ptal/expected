// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_REBINDABLE_HPP
#define BOOST_FUNCTIONAL_REBINDABLE_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <utility>
#include <type_traits>

namespace boost
{
namespace functional
{
namespace category
{
  struct rebindable {};
}
  template <class M>
  struct rebindable_category
  {
    typedef M type;
  };

  template <class M>
  using rebindable_category_t = typename rebindable_category<M>::type;

  template <class T>
  struct rebindable_traits : std::false_type {};
  template <>
  struct rebindable_traits<category::forward>  : std::true_type
  {
    template <class M>
    using value_type = typename M::value_type;

    template <class M, class U>
    using rebind = typename M::template rebind<U>;
  };

  template <class M>
  struct rebindable_traits_t : rebindable_traits<rebindable_category_t<decay_t<M> > > {};

namespace rebindable
{

  template <class M, class Traits = rebindable_traits_t<M> >
  using value_type = typename Traits::template value_type<M>;

  template <class M, class U, class Traits = rebindable_traits_t<M> >
  using rebind = typename Traits::template rebind<M, U>;


}
}
}

#endif // BOOST_FUNCTIONAL_VALUED_HPP
