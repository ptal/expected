// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_MONADS_CATEGORIES_POINTER_LIKE_HPP
#define BOOST_EXPECTED_MONADS_CATEGORIES_POINTER_LIKE_HPP

#include <boost/config.hpp>
#include <boost/functional/monads.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost
{
  namespace monads
  {
    namespace category
    {
      struct pointer_like {};
    }

    template <>
    struct value_traits<category::pointer_like> {
      template <class M>
      using type = typename M::value_type;
      template <class M>
      static constexpr bool has_value(M&& m) { return bool(m); };
      template <class M>
      static constexpr auto deref(M&& m) -> decltype(*m) { return *m; };
    };
  }
}

#endif // BOOST_EXPECTED_MONADS_CATEGORIES_POINTER_LIKE_HPP
