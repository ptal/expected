// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013 Vicente J. Botet Escriba

#ifndef BOOST_THREAD_EXPECTED_HPP
#define BOOST_THREAD_EXPECTED_HPP

#include <boost/thread/detail/config.hpp>
#include <boost/thread/detail/delete.hpp>
#include <boost/thread/detail/move.hpp>

#include <boost/config/abi_prefix.hpp>


namespace boost
{

  struct exceptional_tag {};
  BOOST_CONSTEXPR_OR_CONST exceptional_tag exceptional = {};

  struct emplace_tag {};
  BOOST_CONSTEXPR_OR_CONST emplace_tag emplace = {};

  template <typename ValueType>
  class expected
  {

    BOOST_STATIC_ASSERT_MSG( !boost::is_same<T, exceptional_tag>::value, "bad T" );
    BOOST_STATIC_ASSERT_MSG( !boost::is_same<T, emplace_tag>::value, "bad T" );
  public:
    typedef ValueType value_type;

  private:
    union {
      boost::exception_ptr excpt_;
      value_type& value_; 
    };
    bool has_value_;

  public:
    // Constructors/Destructors/Assignments

    explicit expected(const value_type& rhs) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value_(rhs)))
    : value_(rhs)
    , has_value_(true)
    {}

    explicit expected(BOOST_RV_REF(value_type) rhs)
    : value_(boost::move(rhs))
    , has_value_(true)
    {}

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <class... Args>
    explicit expected(emplace_tag, Args&&... args)
    : value_(boost::forward<Args>(args)...)
    , has_value(true)
    {}
#endif

    expected(const expected& rhs)
    : has_value_(rhs.has_value_) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value_(rhs.value_)))
    {
      if (has_value_)
      {
        ::new (&value_) value_type(rhs.value_);
      }
      else
      {
        ::new (&except_) boost::exception_ptr(rhs.except_);
      }
    }

    expected(BOOST_RV_REF(expected) rhs)
    : has_value_(rhs.has_value_) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(boost::move(rhs.value_)))
    {
      if (has_value_)
      {
        ::new (&value_) value_type(boost::move(rhs.value_));
      }
      else
      {
        ::new (&except_) boost::exception_ptr(boost::move(rhs.except_));
      }
    }

    expected(exceptional_tag, boost::exception_ptr const& p) BOOST_NOEXCEPT
    : except_(p)
    , has_value_(false)
    {}

    /// Requires  typeid(e) == typeid(E)
    template <typename E>
    expected(exceptional_tag, E const& e)
    : except_(boost::make_exception_ptr(e))
    , has_value_(false)
    {}

    expected() BOOST_NOEXCEPT
    : except_(boost::current_exception())
    , has_value_(false)
    {}

    ~expected() // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value_.~value_type()))
    {
      if (valid()) value_.~value_type();
      else except_.boost::exception_ptr::~exception_ptr();
    }

    /// Modifiers
    void swap(expected& rhs) // BOOST_NOEXCEPT_IF( ... )
    {
      if (has_value_)
      {
        if (rhs.has_value_)
        {
          boost::swap(value_, rhs.value_);
        }
        else
        {
          boost::exception_ptr t = boost::move(rhs.except_);
          new (&rhs.value_) T(boost::move(value_));
          new (&except_) boost::exception_ptr(t);
          boost::swap(has_value_, rhs.has_value_);
        }
      }
      else
      {
        if (rhs.has_value_)
        {
          rhs.swap(*this);
        }
        else
        {
          except_.swap(rhs.except_);
        }
      }
    }

    /// Observers
    bool valid() const BOOST_NOEXCEPT
    {
      return has_value_;
    }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    explicit operator bool() const noexcept { return initialized(); }
#endif

    const T& get() const
    {
      if (!valid()) boost::rethrow_exception(except_);
      return value_;
    }
    // Only const access is allowed
//    T& get()
//    {
//      if (!valid()) boost::rethrow_exception(except_);
//      return value_;
//    }

    template <class E>
    bool has_exception() const BOOST_NOEXCEPT
    {
      try
      {
        if (!valid()) boost::rethrow_exception(except_ );
      }
      catch (const E& et)
      {
        return true;
      }
      catch (...)
      {
      }
      return false;
    }

    /// factories
    template <typename F>
    expected<typename boost::result_of<F(const expected&)>::type>
    then(F&& fuct) BOOST_NOEXCEPT
    {
      typedef typename boost::result_of<F(const expected&)>::type result_value_type;
      if (valid())
      {
        return make_expected(funct(value_));
      }
      else
      {
        return make_exceptional_expected<result_value_type>(except_);
      }
    }
  };

  /// Specialized algorithms
  template <class T>
  void swap(expected<T>& x, expected<T>& y) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(x.swap(y)))
  {
    x.swap(y);
  }

  /// Factories
  template <typename T>
  expected<T> make_expected(const T& v) //BOOST_NOEXCEPT
  {
    return expected<T>(v);
  }
  template <typename T>
  expected<T> make_expected(BOOST_RV_REF(T) v) //BOOST_NOEXCEPT
  {
    return expected<T>(boost::forward<T>(v));
  }
  template <typename T>
  expected<T> make_exceptional_expected(exceptional, boost::exception_ptr const& p) BOOST_NOEXCEPT
  {
    return expected<T>(p);
  }

  /// Requires  typeid(e) == typeid(E)
  template <typename T, typename E>
  expected<T> make_exceptional_expected(E const& e) BOOST_NOEXCEPT
  {
    return expected<T>(exceptional, e);
  }

  template <typename T>
  expected<T> make_exceptional_expected() BOOST_NOEXCEPT
  {
    return expected<T>();
  }

  template <typename F>
  expected<typename boost::result_of<F()>::type>
  make_noexcept_expected(F&& fuct) BOOST_NOEXCEPT
  {
    try {
      return make_expected(fun());
    } catch (...) {
      return make_exceptional_expected();
    }
  }

#if 0
  // Example
  expected<int> parseInt(const std::string& s)
  {
    int result;
    //...
    if (nonDigit)
    {
      return make_exceptional_expected<int>(
          std::invalid_argument("not a number"));
    }
    //...
    if (tooManyDigits)
    {
      return make_exceptional_expected<int>(
          std::out_of_range("overflow"));
    }
    //...
    return make_expected(result);
  }

  // Caller
  std::string s = readline();
  auto x = parseInt(s).get(); // throw on error
  auto y = parseInt(s); // won�t throw
  if (!y.valid())
  {
    // handle locally
    if (y.has_exception<std::invalid_argument>())
    {
      // no digits
      //...
    }
    y.get(); // just "re"throw
  }


  /// Chaining
  make_noexcept_expected(doSomething).then(doSomethingElse).then(doSomethingMore)

#endif

}

#include <boost/config/abi_suffix.hpp>

#endif
