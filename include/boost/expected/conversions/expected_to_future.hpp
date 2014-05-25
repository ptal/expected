// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_TO_FUTURE_HPP
#define BOOST_EXPECTED_EXPECTED_TO_FUTURE_HPP

#include <boost/config.hpp>
#include <boost/expected/expected.hpp>
//#include <boost/thread/future.hpp>
#include <future>

namespace boost
{
  template <class V>
  expected<std::exception_ptr, V> make_expected(std::future<V>&& f) {
    //assert (f.ready() && "future not ready");
    try {
      return f.get();
    } catch (...) {
      return make_unexpected_from_current_exception();
    }
  }

  template <class V, class E>
  std::future<V> make_unexpected_future(E e)  {
    std::promise<V> p;
    std::future<V> f = p.get_future();
    p.set_exception(std::make_exception_ptr(e));
    return std::move(f);
  }

  template <class V, class T>
  std::future<T> make_ready_future(expected<std::exception_ptr, V, T> e) {
    if (e.valid()) return make_ready_future(*e);
    else return make_unexpected_future<V>(e.error());
  }



} // namespace boost

#endif // BOOST_EXPECTED_UNEXPECTED_HPP
