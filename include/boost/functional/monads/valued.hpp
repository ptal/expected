// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_VALUED_HPP
#define BOOST_FUNCTIONAL_VALUED_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/monads/rebindable.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <utility>
#include <stdexcept>
#include <type_traits>

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
  using namespace ::boost::functional::rebindable;

  template <class M>
  struct value_category
  {
    typedef M type;
  };

  template <class M>
  using value_category_t = typename value_category<M>::type;

  // bad_access exception class.
  class bad_access : public std::logic_error
  {
    public:
      bad_access()
      : std::logic_error("Bad access on PossibleValued object.")
      {}

      // todo - Add implicit/explicit conversion to error_type ?
  };

  template <class T>
  struct valued_traits  : std::false_type {};
  template <>
  struct valued_traits<category::forward> : std::true_type
  {
    template <class M>
    static constexpr bool has_value(M&& m)
    { return m.has_value();}

    template <class M>
    static constexpr auto deref(M&& m) -> decltype(m.deref())
    { return m.deref();};

    template <class M>
    static constexpr auto get_value(M&& m) -> decltype(m.value())
    { return m.value(); };
  };

  template <class M>
  struct valued_traits_t : valued_traits<value_category_t<decay_t<M> > > {};

  template <class M, class Traits = valued_traits_t<M> >
  constexpr auto
  has_value(M&& e) -> decltype(Traits::has_value(std::forward<M>(e)))
  {
    return Traits::has_value(std::forward<M>(e));
  }

  template <class M, class Traits = valued_traits_t<M> >
  constexpr auto
  deref(M&& e) -> decltype(Traits::deref(std::forward<M>(e)))
  {
    return Traits::deref(std::forward<M>(e));
  }

//  template <class M, class Traits = valued_traits_t<M> >
//  static constexpr auto
//  value(M&& e) -> decltype(Traits::get_value(std::forward<M>(e)))
//  {
//    return Traits::get_value(std::forward<M>(e));
//  }

}
}
}

#endif // BOOST_FUNCTIONAL_VALUED_HPP
