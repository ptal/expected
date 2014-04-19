// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_VALUED_HPP
#define BOOST_FUNCTIONAL_VALUED_HPP

#include <boost/functional/type_traits_t.hpp>
#include <utility>

namespace boost
{
namespace functional
{
namespace category
{
  struct valued {};
}
namespace valued
{

  template <class M>
  struct value_category
  {
    typedef M type;
  };

  template <class M>
  using value_category_t = typename value_category<M>::type;

namespace detail
{
  template <class T, class = void>
  struct value_traits_h {};
  template <class T>
  struct value_traits_h<T,
    void_t<
      typename T::value_type,
      decltype(std::declval<T>().has_value()),
      decltype(std::declval<T>().deref())
    >
  >
  {
    constexpr static bool value = true;

    template <class M>
    using value_type = typename M::value_type;

    template <class M>
    static constexpr bool has_value(M&& m)
    { return m.has_value();}

    template <class M>
    static constexpr auto deref(M&& m) -> decltype(m.deref())
    { return m.deref();};
  };

}

  template <class T>
  struct value_traits : detail::value_traits_h<T> {};

  template <class M>
  struct value_traits_t : value_traits<value_category_t<decay_t<M> > > {};

  template <class M, class Traits = value_traits_t<M> >
  using value_type_t = typename Traits::template value_type<M>;

  template <class M, class Traits = value_traits_t<M> >
  constexpr auto
  has_value(M&& e) -> decltype(Traits::has_value(std::forward<M>(e)))
  {
    return Traits::has_value(std::forward<M>(e));
  }
  template <class M, class Traits = value_traits_t<M> >
  constexpr auto
  deref(M&& e) -> decltype(Traits::deref(std::forward<M>(e)))
  {
    return Traits::deref(std::forward<M>(e));
  }
}
}
}

#endif // BOOST_FUNCTIONAL_VALUED_HPP
