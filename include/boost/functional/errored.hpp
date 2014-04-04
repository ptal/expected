// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_ERRORED_HPP
#define BOOST_FUNCTIONAL_ERRORED_HPP

//#include <boost/config.hpp>
//#include <boost/functional/valued.hpp>
#include <boost/functional/type_traits_t.hpp>
//#include <exception>
#include <utility>
//#include <type_traits>

namespace boost
{
  namespace monads
  {

    template <class M>
    struct unexpected_category {
      typedef M type;
    };

    template <class M>
    using unexpected_category_t = typename unexpected_category<M>::type;

    template <class T>
    struct unexpected_traits {
      template <class M>
      using type = typename M::unexpected_type;
      template <class M>
      static constexpr auto get_unexpected(M&& m) -> decltype(m.get_unexpected()) { return m.get_unexpected(); };
      template <class M>
      static constexpr auto error(M&& m) -> decltype(m.error()) { return m.error(); };
    };

    template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
    using unexpected_type_t = typename Traits::template type<M>;

    template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
    static constexpr auto
    get_unexpected(M&& e) -> decltype(Traits::get_unexpected(std::forward<M>(e)))
    {
      return Traits::get_unexpected(std::forward<M>(e));
    }
    template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
    static constexpr  auto
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

#endif // BOOST_FUNCTIONAL_ERRORED_HPP
