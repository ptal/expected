// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_REBINDABLE_HPP
#define BOOST_FUNCTIONAL_REBINDABLE_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <boost/functional/monads/categories/default.hpp>
#include <boost/functional/meta.hpp>
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

  template <class T>
  struct rebindable_traits : std::false_type {};

  template <class M>
  struct is_rebindable : rebindable_traits<M> {};

  template <class M> using if_rebindable =
      typename std::enable_if<is_rebindable<M>::value, rebindable_traits<M> >::type;

namespace rebindable
{
  template <class M, class Traits = if_rebindable<M>>
  using value_type = typename Traits::template value_type<M>;

  template <class M, class Traits = if_rebindable<M> >
  using type_constructor = typename Traits::template type_constructor<M>;

#ifdef BOOST_MSVC
  // Work around ICE in VS2013 and VS14 CTP 3
  template <class M, class U, class Traits = if_rebindable<M> >
  struct rebind
    : public rebindable_traits<M>::template rebind<M, U>
  {
  };
#else
  template <class M, class U, class Traits = if_rebindable<M> >
  using rebind = typename Traits::template rebind<M, U>;
#endif
}

template <>
struct rebindable_traits<category::default_>  : std::true_type {
#ifdef BOOST_MSVC
  // Work around ICE in VS2013 and VS14 CTP 3
  template <class M, class U>
  struct rebind
    : public apply<rebindable::type_constructor<M>, U>
  {
  };
#else
  template <class M, class U>
  using rebind = apply<rebindable::type_constructor<M>,U>;
#endif

};
template <>
struct rebindable_traits<category::forward>  : rebindable_traits<category::default_>
{
  template <class M>
  using value_type = typename M::value_type;

  template <class M>
  using type_constructor = typename M::type_constructor;
};

}
}

#endif // BOOST_FUNCTIONAL_VALUED_HPP
