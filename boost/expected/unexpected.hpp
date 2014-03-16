// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_UNEXPECTED_HPP
#define BOOST_EXPECTED_UNEXPECTED_HPP

#include <exception>
#include <boost/exception_ptr.hpp>

namespace boost
{
  template <typename ErrorType = std::exception_ptr>
  class unexpected_type
  {
    ErrorType error_;
  public:
    unexpected_type() = delete;

    BOOST_CONSTEXPR explicit unexpected_type(ErrorType e) :
      error_(e)
    {
    }
    BOOST_CONSTEXPR
    ErrorType value() const
    {
      return error_;
    }
  };

  template <class E>
  BOOST_CONSTEXPR unexpected_type<E> make_unexpected(E ex)
  {
    return unexpected_type<E> (ex);
  }

  template <>
  struct unexpected_type<boost::exception_ptr>
  {
    boost::exception_ptr error_;
  public:
    unexpected_type() = delete;

    explicit unexpected_type(boost::exception_ptr e) :
      error_(e)
    {
    }

    template <class E> explicit unexpected_type(E e) :
      error_(boost::copy_exception(e))
    {
    }

    boost::exception_ptr value() const
    {
      return error_;
    }
  };

  template <>
  struct unexpected_type<std::exception_ptr>
  {
    std::exception_ptr error_;
  public:
    unexpected_type() = delete;

    explicit unexpected_type(std::exception_ptr e) :
      error_(e)
    {
    }

    template <class E> explicit unexpected_type(E e) :
      error_(std::make_exception_ptr(e))
    {
    }
    std::exception_ptr value() const
    {
      return error_;
    }
  };

  inline unexpected_type<std::exception_ptr> make_unexpected_from_current_exception()
  {
    return unexpected_type<std::exception_ptr> (std::current_exception());
  }

} // namespace boost

#endif // BOOST_EXPECTED_UNEXPECTED_HPP
