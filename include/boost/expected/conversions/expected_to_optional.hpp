// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_TO_OPTIONAL_HPP
#define BOOST_EXPECTED_EXPECTED_TO_OPTIONAL_HPP

#include <boost/expected/expected.hpp>
#include <boost/optional.hpp>
//#include <optional>

namespace boost
{
  struct conversion_from_nullopt {};

  template <class V>
  expected<std::exception_ptr, V> make_expected(optional<V> v) {
    if (v) return make_expected(*v);
    else make_unexpected(conversion_from_nullopt());
  }

  template <class V, class T>
  optional<V> make_optional(expected<std::exception_ptr, V, T> e) {
    if (e.valid()) return optional<V>(*e);
    else return none;
  }

} // namespace boost

#endif // BOOST_EXPECTED_UNEXPECTED_HPP
