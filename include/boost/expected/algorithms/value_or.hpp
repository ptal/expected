// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_ALGORITHMS_VALUE_OR_HPP
#define BOOST_EXPECTED_ALGORITHMS_VALUE_OR_HPP

#include <boost/expected/expected.hpp>

namespace boost
{
namespace expected_alg
{

  template <class T>
  struct just_t
  {
    just_t(T v) :
      value_(v)
    {
    }
    template <class ...A>
    T operator()(A...)
    { return value_;}
  private:
    T value_;
  };

  template <class T>
  inline just_t<decay_t<T>> just(T&& v)
  {
    return just_t<decay_t<T>>(std::forward<T>(v));
  }

  template <class V, class E, class T>
  BOOST_CONSTEXPR V value_or(expected<E,V,T> const& e, BOOST_FWD_REF(V) v)
  {
    // We are sure that e.catch_error(just(std::forward<V>(v))) will be valid or a exception will be thrown
    // so the dereference is safe
    return * e.catch_error(just(std::forward<V>(v)));
  }

  template <class V, class E, class T>
  BOOST_CONSTEXPR V value_or(expected<E,V,T> && e, BOOST_FWD_REF(V) v)
  {
    // We are sure that e.catch_error(just(std::forward<V>(v))) will be valid or a exception will be thrown
    // so the dereference is safe
    return * e.catch_error(just(std::forward<V>(v)));
  }

} // namespace expected_alg
} // namespace boost

#endif // BOOST_EXPECTED_HPP
