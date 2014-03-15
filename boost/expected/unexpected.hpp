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
  struct unexpected_type
  {
    ErrorType error_;

    unexpected_type() :
      error_()
    {
    }
    ;
    explicit unexpected_type(ErrorType e) :
      error_(e)
    {
    }
    ;
  };

  template <class E>
  inline unexpected_type<E> make_unexpected(E ex)
  {
    return unexpected_type<E> (ex);
  }

  template <>
  struct unexpected_type<boost::exception_ptr>
  {
    boost::exception_ptr error_;
    unexpected_type() :
      error_(boost::current_exception())
    {
    }
    explicit unexpected_type(boost::exception_ptr e) :
      error_(e)
    {
    }
    template <class E> explicit unexpected_type(E e) :
      error_(boost::copy_exception(e))
    {
    }
  };

  template <>
  struct unexpected_type<std::exception_ptr>
  {
    std::exception_ptr error_;
    unexpected_type() :
      error_(std::current_exception())
    {
    }
    explicit unexpected_type(std::exception_ptr e) :
      error_(e)
    {
    }
    template <class E> explicit unexpected_type(E e) :
      error_(std::make_exception_ptr(e))
    {
    }
  };

} // namespace boost

#endif // BOOST_EXPECTED_UNEXPECTED_HPP
