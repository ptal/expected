// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013,2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_DETAIL_STATIC_ADDRESSOF_HPP
#define BOOST_EXPECTED_DETAIL_STATIC_ADDRESSOF_HPP

#include <boost/expected/config.hpp>
#include <boost/expected/detail/requires.hpp>

#ifdef BOOST_EXPECTED_USE_BOOST_HPP
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#endif
#include <memory>
#include <utility>

namespace boost {
namespace detail {

#if defined BOOST_EXPECTED_USE_STD_ADDRESSOF

template <typename T>
BOOST_CONSTEXPR T* static_addressof(T& ref)
{
  return std::addressof(ref);
}

#else

template <typename T>
struct has_overloaded_addressof
{
  template <class X>
  static BOOST_CONSTEXPR bool has_overload(...) { return false; }

  template <class X, size_t S = sizeof(std::declval< X&>().operator&()) >
  static BOOST_CONSTEXPR bool has_overload(bool) { return true; }

  static BOOST_CONSTEXPR_OR_CONST bool value = has_overload<T>(true);
};

template <typename T>
BOOST_CONSTEXPR T* static_addressof(T& ref,
    BOOST_EXPECTED_REQUIRES(!has_overloaded_addressof<T>::value))
{
  return &ref;
}

template <typename T>
BOOST_CONSTEXPR T* static_addressof(T& ref,
    BOOST_EXPECTED_REQUIRES(has_overloaded_addressof<T>::value))
{
  return std::addressof(ref);
}

#endif

} // namespace detail
} // namespace boost

#endif // BOOST_EXPECTED_DETAIL_STATIC_ADDRESSOF_HPP
