// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_VALUED_HPP
#define BOOST_FUNCTIONAL_VALUED_HPP

#include <boost/functional/type_traits_t.hpp>
#include <boost/functional/monads/rebindable.hpp>
#include <boost/functional/monads/categories/forward.hpp>
#include <boost/functional/monads/categories/default.hpp>
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

  template <class M>
  struct valued_category
  {
    typedef M type;
  };

  template <class M>
  using valued_category_t = typename valued_category<M>::type;

  template <class T>
  struct valued_traits  : std::false_type {};
  template <>
  struct valued_traits<category::default_> : std::true_type {};
  template <>
  struct valued_traits<category::forward> : valued_traits<category::default_>
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
  struct valued_traits_t : valued_traits<valued_category_t<decay_t<M> > > {};

  template <class M>
  struct is_valued : std::integral_constant<bool, is_rebindable<M>::value &&
    valued_traits<valued_category_t<M>>::value
  >
  {};

namespace valued
{
  using namespace ::boost::functional::rebindable;
  // bad_access exception class.
  class bad_access : public std::logic_error
  {
    public:
      bad_access()
      : std::logic_error("Bad access on PossibleValued object.")
      {}

      // todo - Add implicit/explicit conversion to error_type ?
  };

  template <class M, class Traits = valued_traits_t<M>, class = std::enable_if<is_valued<decay_t<M>>::value> >
  constexpr auto
  has_value(M&& e) -> decltype(Traits::has_value(std::forward<M>(e)))
  {
    return Traits::has_value(std::forward<M>(e));
  }

  template <class M, class Traits = valued_traits_t<M>, class = std::enable_if<is_valued<decay_t<M>>::value> >
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
