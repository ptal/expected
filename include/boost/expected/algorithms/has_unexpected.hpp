// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_ALGORITHMS_HAS_UNEXPECTED_HPP
#define BOOST_EXPECTED_ALGORITHMS_HAS_UNEXPECTED_HPP

#include <boost/expected/expected.hpp>

namespace boost
{
namespace expected_alg
{

  template <class Ex, class V, class T>
  bool has_unexpected(expected<std::exception_ptr, V, T> const& e)
  {
    try {
      if(!e) std::rethrow_exception(e.error());
    }
    catch(Ex& e)
    {
      return true;
    }
    catch(...)
    {
    }
    return false;
  }

  template <class Ex, class V, class T>
  bool has_unexpected(expected<boost::exception_ptr, V, T> const& e)
  {
    try {
      if(!e) boost::rethrow_exception(e.error());
    }
    catch(Ex& e)
    {
      return true;
    }
    catch(...)
    {
    }
    return false;
  }

} // namespace expected_alg
} // namespace boost

#endif // BOOST_EXPECTED_ALGORITHMS_HAS_UNEXPECTED_HPP
