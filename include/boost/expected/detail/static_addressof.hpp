// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013,2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_DETAIL_STATIC_ADDRESSOF_HPP
#define BOOST_EXPECTED_DETAIL_STATIC_ADDRESSOF_HPP

#include <boost/expected/config.hpp>

#include <boost/move/move.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

//#include <stdexcept>
#include <utility>


# define REQUIRES(...) typename ::boost::enable_if_c<__VA_ARGS__, void*>::type = 0
# define T_REQUIRES(...) typename = typename ::boost::enable_if_c<(__VA_ARGS__)>::type

namespace boost {
namespace detail {

template <typename T>
struct has_overloaded_addressof
{
  template <class X>
  static BOOST_EXPECTED_CONSTEXPR bool has_overload(...) { return false; }

  template <class X, size_t S = sizeof(std::declval< X&>().operator&()) >
  static BOOST_EXPECTED_CONSTEXPR bool has_overload(bool) { return true; }

  static BOOST_EXPECTED_CONSTEXPR_OR_CONST bool value = has_overload<T>(true);
};

template <typename T>
BOOST_EXPECTED_RELAXED_CONSTEXPR T* static_addressof(T& ref,
  REQUIRES(!has_overloaded_addressof<T>::value))
{
  return &ref;
}

template <typename T>
BOOST_EXPECTED_RELAXED_CONSTEXPR T* static_addressof(T& ref,
  REQUIRES(has_overloaded_addressof<T>::value))
{
  return std::addressof(ref);
}

} // namespace detail
} // namespace boost

#undef REQUIRES
#undef T_REQUIRES


#endif // BOOST_EXPECTED_DETAIL_STATIC_ADDRESSOF_HPP
