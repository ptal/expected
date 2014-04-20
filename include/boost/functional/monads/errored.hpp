// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_ERRORED_HPP
#define BOOST_FUNCTIONAL_ERRORED_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/monads/valued.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <utility>
#include <type_traits>

namespace boost
{
namespace functional
{
  template <class M>
  struct unexpected_category
  {
    typedef M type;
  };

  template <class M>
  using unexpected_category_t = typename unexpected_category<M>::type;

  template <class T>
  struct unexpected_traits  : std::false_type {};

  template <>
  struct unexpected_traits<category::forward>  : std::true_type
  {
    template <class M>
    using unexpected_type_type = typename M::unexpected_type_type;

    template <class M>
    static constexpr auto get_unexpected(M&& m) -> decltype(m.get_unexpected())
    { return m.get_unexpected();};

    template <class M>
    static constexpr auto error(M&& m) -> decltype(m.error())
    { return m.error();};
  };

namespace errored
{
  using namespace ::boost::functional::valued;

  template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
  using unexpected_type_t = typename Traits::template unexpected_type_type<M>;

  template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
  static constexpr auto
  get_unexpected(M&& e) -> decltype(Traits::get_unexpected(std::forward<M>(e)))
  {
    return Traits::get_unexpected(std::forward<M>(e));
  }
  template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
  static constexpr auto
  error(M&& e) -> decltype(Traits::error(std::forward<M>(e)))
  {
    return Traits::error(std::forward<M>(e));
  }
//    template <class M, class E, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
//    static constexpr M
//    make_unexpected(E&& e) -> decltype(Traits::make_unexpected(std::forward<M>(e)))
//    {
//      return Traits::make_unexpected(std::forward<M>(e));
//    }

}
}
}

#endif // BOOST_FUNCTIONAL_ERRORED_HPP
