// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_MONADS_ALGORITHMS_HAVE_VALUE_HPP
#define BOOST_EXPECTED_MONADS_ALGORITHMS_HAVE_VALUE_HPP

#include <boost/config.hpp>
#include <boost/functional/monads.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

#define REQUIRES(...) typename ::boost::enable_if_c<__VA_ARGS__, void*>::type = 0
#define T_REQUIRES(...) typename = typename ::boost::enable_if_c<(__VA_ARGS__)>::type

namespace boost
{
  namespace monads
  {

    template< class M >
    BOOST_CONSTEXPR bool have_value( M&& m )
    {
      return has_value(std::forward<M>(m));
    }
    template< class M1, class ...Ms >
    BOOST_CONSTEXPR bool have_value( M1&& m1, Ms&& ...ms )
    {
      return has_value(std::forward<M1>(m1)) && have_value( std::forward<Ms>(ms)... );
    }

  }
}

#undef REQUIRES
#undef T_REQUIRES

#endif // BOOST_EXPECTED_MONADS_ALGORITHMS_HAVE_VALUE_HPP
