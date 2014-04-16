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
    namespace valued
    {

      template <class M>
      struct value_category
      {
        typedef M type;
      };

      template <class M>
      using value_category_t = typename value_category<M>::type;

      template <class T>
      struct value_traits
      {
        template <class M>
        using type = typename M::value_type;

        template <class M>
        static constexpr bool has_value(M&& m)
        { return m.has_value();}

        template <class M>
        static constexpr auto deref(M&& m) -> decltype(m.deref())
        { return m.deref();};
      };

      template <class M, class Traits = value_traits<value_category_t<decay_t<M> > > >
      using value_type_t = typename Traits::template type<M>;

      template <class M, class Traits = value_traits<value_category_t<decay_t<M> > > >
      static constexpr auto
      has_value(M&& e) -> decltype(Traits::has_value(std::forward<M>(e)))
      {
        return Traits::has_value(std::forward<M>(e));
      }
      template <class M, class Traits = value_traits<value_category_t<decay_t<M> > > >
      static constexpr auto
      deref(M&& e) -> decltype(Traits::deref(std::forward<M>(e)))
      {
        return Traits::deref(std::forward<M>(e));
      }
    }
  }
}

#endif // BOOST_FUNCTIONAL_VALUED_HPP
