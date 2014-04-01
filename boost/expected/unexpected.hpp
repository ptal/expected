// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_UNEXPECTED_HPP
#define BOOST_EXPECTED_UNEXPECTED_HPP

#include <exception>
#include <utility>
#include <boost/exception_ptr.hpp>
#include <boost/functional/type_traits_t.hpp>
#include <boost/type_traits.hpp>

namespace boost
{

  template <typename ErrorType = std::exception_ptr>
  class unexpected_type
  {
    ErrorType error_;
  public:
    unexpected_type() = delete;

    BOOST_FORCEINLINE BOOST_CONSTEXPR explicit unexpected_type(ErrorType const& e) :
      error_(e)
    {
    }
    BOOST_FORCEINLINE BOOST_CONSTEXPR explicit unexpected_type(ErrorType&& e) :
      error_(std::move(e))
    {
    }
    BOOST_CONSTEXPR
    BOOST_FORCEINLINE ErrorType const& value() const
    {
      return error_;
    }
  };

  template <class E>
  BOOST_FORCEINLINE BOOST_CONSTEXPR unexpected_type<decay_t<E> > make_unexpected(E&& ex)
  {
    return unexpected_type<decay_t<E>> (ex);
  }

  template <>
  struct unexpected_type<boost::exception_ptr>
  {
    boost::exception_ptr error_;
  public:
    unexpected_type() = delete;

    BOOST_FORCEINLINE explicit unexpected_type(boost::exception_ptr const& e) :
      error_(e)
    {
    }
    BOOST_FORCEINLINE explicit unexpected_type(boost::exception_ptr&& e) :
      error_(std::move(e))
    {
    }

    template <class E>
    BOOST_FORCEINLINE explicit unexpected_type(E e) :
      error_(boost::copy_exception(e))
    {
    }

    BOOST_FORCEINLINE boost::exception_ptr const& value() const
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

    BOOST_FORCEINLINE explicit unexpected_type(std::exception_ptr const& e) :
      error_(e)
    {
    }

    BOOST_FORCEINLINE explicit unexpected_type(std::exception_ptr &&e) :
      error_(std::move(e))
    {
    }

    template <class E>
    BOOST_FORCEINLINE explicit unexpected_type(E e) :
      error_(std::make_exception_ptr(e))
    {
    }
    BOOST_FORCEINLINE std::exception_ptr const& value() const
    {
      return error_;
    }
  };

  template <class E>
  BOOST_CONSTEXPR bool operator==(const unexpected_type<E>& x, const unexpected_type<E>& y)
  {
    return x.value() == y.value();
  }
  template <class E>
  BOOST_CONSTEXPR bool operator!=(const unexpected_type<E>& x, const unexpected_type<E>& y)
  {
    return !(x == y);
  }

  template <class E>
  BOOST_CONSTEXPR bool operator<(const unexpected_type<E>& x, const unexpected_type<E>& y)
  {
    return x.value() < y.value();
  }

  template <class E>
  BOOST_CONSTEXPR bool operator>(const unexpected_type<E>& x, const unexpected_type<E>& y)
  {
    return (y < x);
  }

  template <class E>
  BOOST_CONSTEXPR bool operator<=(const unexpected_type<E>& x, const unexpected_type<E>& y)
  {
    return !(y < x);
  }

  template <class E>
  BOOST_CONSTEXPR bool operator>=(const unexpected_type<E>& x, const unexpected_type<E>& y)
  {
    return !(x < y);
  }

  inline BOOST_CONSTEXPR bool operator<(const unexpected_type<std::exception_ptr>& x, const unexpected_type<std::exception_ptr>& y)
  {
    return false;
  }
  inline BOOST_CONSTEXPR bool operator>(const unexpected_type<std::exception_ptr>& x, const unexpected_type<std::exception_ptr>& y)
  {
    return false;
  }
  inline BOOST_CONSTEXPR bool operator<=(const unexpected_type<std::exception_ptr>& x, const unexpected_type<std::exception_ptr>& y)
  {
    return x==y;
  }
  inline BOOST_CONSTEXPR bool operator>=(const unexpected_type<std::exception_ptr>& x, const unexpected_type<std::exception_ptr>& y)
  {
    return x==y;
  }

  inline BOOST_CONSTEXPR bool operator<(const unexpected_type<boost::exception_ptr>& x, const unexpected_type<boost::exception_ptr>& y)
  {
    return false;
  }
  inline BOOST_CONSTEXPR bool operator>(const unexpected_type<boost::exception_ptr>& x, const unexpected_type<boost::exception_ptr>& y)
  {
    return false;
  }
  inline BOOST_CONSTEXPR bool operator<=(const unexpected_type<boost::exception_ptr>& x, const unexpected_type<boost::exception_ptr>& y)
  {
    return x==y;
  }
  inline BOOST_CONSTEXPR bool operator>=(const unexpected_type<boost::exception_ptr>& x, const unexpected_type<boost::exception_ptr>& y)
  {
    return x==y;
  }

  template <typename E>
  struct is_unexpected : false_type {};
  template <typename E>
  struct is_unexpected<unexpected_type<E> > : true_type {};

  BOOST_FORCEINLINE unexpected_type<std::exception_ptr> make_unexpected_from_current_exception()
  {
    return unexpected_type<std::exception_ptr> (std::current_exception());
  }

} // namespace boost

#endif // BOOST_EXPECTED_UNEXPECTED_HPP
