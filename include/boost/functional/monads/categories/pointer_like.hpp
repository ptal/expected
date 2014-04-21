// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_MONADS_CATEGORIES_POINTER_LIKE_HPP
#define BOOST_EXPECTED_MONADS_CATEGORIES_POINTER_LIKE_HPP

#include <boost/functional/monads/valued.hpp>
#include <type_traits>

namespace boost
{
namespace functional
{
  namespace category
  {
    struct pointer_like {};
  }

  template <class T>
  struct rebindable_traits<T*> : std::true_type
  {
    template <class M>
    using value_type = T;

    template <class M, class U>
    using rebind = U*;
  };

  template <>
  struct valued_traits<category::pointer_like> : std::true_type
  {
    template <class M>
    static constexpr bool has_value(M&& m) { return bool(m); }

    template <class M>
    static constexpr auto deref(M&& m) -> decltype(*m) { return *m; }

    template <class M>
    static constexpr value_type<M> get_value(M&& m)
    {
      return (m) ? *m : throw bad_access(),*m;
    }
  };

}
}

#endif // BOOST_EXPECTED_MONADS_CATEGORIES_POINTER_LIKE_HPP
