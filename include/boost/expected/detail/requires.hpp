// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2015 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_DETAIL_REQUIRES_HPP
#define BOOST_EXPECTED_DETAIL_REQUIRES_HPP

#include <type_traits>

namespace boost {
  namespace expected_detail {
      enum class enabler {};
  }
}

# define BOOST_EXPECTED_REQUIRES(...) typename std::enable_if<__VA_ARGS__, void*>::type = 0
# define BOOST_EXPECTED_T_REQUIRES(...) typename = typename std::enable_if<(__VA_ARGS__), boost::expected_detail::enabler>::type

// This is needed to make the condition dependent
# define BOOST_EXPECTED_0_REQUIRES(...) \
  template< \
  bool B = (__VA_ARGS__), \
  typename std::enable_if<B, int>::type = 0 \
  >

#endif // header
