// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_ALGORITHMS_UNWRAP_HPP
#define BOOST_EXPECTED_ALGORITHMS_UNWRAP_HPP

#include <boost/expected/expected.hpp>

//decay_t

namespace boost
{
namespace expected_alg
{

  // Factories
  // unwrap and if_then_else factories could be added as member functions
  template <class V, class E, class T1, class T2>
  expected<E,V,T1> unwrap(expected<E, expected<E,V, T1>, T2> ee) {
    if (ee) return *ee;
    return ee.get_unexpected();
  }
  template <class V, class E, class T>
  expected<E,V,T> unwrap(expected<E,V,T> e) {
    return e;
  }

} // namespace expected_alg
} // namespace boost

#endif // BOOST_EXPECTED_ALGORITHMS_UNWRAP_HPP
