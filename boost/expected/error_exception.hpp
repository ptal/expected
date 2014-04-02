// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_ERROR_EXCEPTION_HPP
#define BOOST_EXPECTED_ERROR_EXCEPTION_HPP

#include <boost/expected/expected.hpp>

namespace boost {

template <class Error, class Exception>
struct error_exception {
    typedef Error error_type;
    typedef Exception exception_type;
    error_exception() : value_(){}
    error_exception(Error e) : value_(e){}
    operator Error() { return value_; }
    Error value() { return value_; }
private:
    Error value_;
};

template <class E, class X>
BOOST_CONSTEXPR bool operator==(const error_exception<E,X>& x, const error_exception<E,X>& y)
{
  return x.value() == y.value();
}
template <class E, class X>
BOOST_CONSTEXPR bool operator!=(const error_exception<E,X>& x, const error_exception<E,X>& y)
{
  return !(x == y);
}

template <class E, class X>
BOOST_CONSTEXPR bool operator<(const error_exception<E,X>& x, const error_exception<E,X>& y)
{
  return x.value() < y.value();
}

template <class E, class X>
BOOST_CONSTEXPR bool operator>(const error_exception<E,X>& x, const error_exception<E,X>& y)
{
  return (y < x);
}

template <class E, class X>
BOOST_CONSTEXPR bool operator<=(const error_exception<E,X>& x, const error_exception<E,X>& y)
{
  return !(y < x);
}

template <class E, class X>
BOOST_CONSTEXPR bool operator>=(const error_exception<E,X>& x, const error_exception<E,X>& y)
{
  return !(x < y);
}

// Specialization for error_exception
template <class ErrorType, class Exception>
struct expected_traits<error_exception<ErrorType, Exception> >
: expected_error_traits<ErrorType, Exception >
{
};


} // namespace boost

#endif // BOOST_EXPECTED_ERROR_EXCEPTION_HPP
