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
  struct unexpected
  {
    ErrorType error_;

    unexpected() :
      error_()
    {
    }
    ;
    explicit unexpected(ErrorType e) :
      error_(e)
    {
    }
    ;
  };

  template <class E>
  inline unexpected<E> make_unexpected(E ex)
  {
    return unexpected<E> (ex);
  }

  template <>
  struct unexpected<boost::exception_ptr>
  {
    boost::exception_ptr error_;
    unexpected() :
      error_(boost::current_exception())
    {
    }
    explicit unexpected(boost::exception_ptr e) :
      error_(e)
    {
    }
    template <class E> explicit unexpected(E e) :
      error_(boost::copy_exception(e))
    {
    }
  };

  template <>
  struct unexpected<std::exception_ptr>
  {
    std::exception_ptr error_;
    unexpected() :
      error_(std::current_exception())
    {
    }
    explicit unexpected(std::exception_ptr e) :
      error_(e)
    {
    }
    template <class E> explicit unexpected(E e) :
      error_(std::make_exception_ptr(e))
    {
    }
  };

} // namespace boost

#endif // BOOST_EXPECTED_UNEXPECTED_HPP
