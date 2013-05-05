// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013 Vicente J. Botet Escriba
// (C) Copyright 2013 Pierre Talbot

#ifndef BOOST_EXPECTED_HPP
#define BOOST_EXPECTED_HPP

#include <stdexcept>

#include <boost/config.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/move/move.hpp>

namespace boost
{
  // bad_expected_access exception class.
  template <class Error>
  class bad_expected_access : public std::logic_error
  {
    public:
      typedef Error exceptional_type;
    private:
      exceptional_type error_value;
    public:
      bad_expected_access(const Error& e)
      : std::logic_error("Found an error instead of the expected value.")
      , error_value(e)
      {}

      exceptional_type& error() { return error_value; }
      const exceptional_type& error() const { return error_value; }

      // Add implicit/explicit conversion to exceptional_type ?
  };

  // Traits classes
  template <typename ExceptionalType>
  struct exceptional_traits
  {
    typedef ExceptionalType exceptional_type;

    template <class E>
    static exceptional_type make_exceptional(E const& e)
    {
      return exceptional_type(e);
    }
    
    static exceptional_type catch_exception()
    {
      boost::rethrow_exception(boost::current_exception());
    }
    
    static void bad_access(const exceptional_type &e)
    {
      boost::throw_exception(bad_expected_access<exceptional_type>(e)); 
    }
  };

  // Specialization for exception_ptr
  template <>
  struct exceptional_traits<boost::exception_ptr>
  {
    typedef boost::exception_ptr exceptional_type;

    template <class E>
    static exceptional_type make_exceptional(E const& e)
    {
      return boost::copy_exception(e);
    }
    
    static exceptional_type catch_exception()
    {
      return boost::current_exception();
    }
    
    static void bad_access(const exceptional_type &e)
    {
      boost::rethrow_exception(e);
    }
  };

  struct exceptional_tag {};
  BOOST_CONSTEXPR_OR_CONST exceptional_tag exceptional = {};

  struct emplace_tag {};
  BOOST_CONSTEXPR_OR_CONST emplace_tag emplace = {};

  template <typename ValueType, typename ExceptionalType=boost::exception_ptr>
  class expected
  {
  public:
    typedef ValueType value_type;
    typedef ExceptionalType exceptional_type;
    typedef exceptional_traits<exceptional_type> traits_type;
    typedef expected<value_type, exceptional_type> this_type;

  private:

    // Static asserts.
    typedef boost::is_same<value_type, exceptional_tag> is_same_value_exceptional_tag;
    typedef boost::is_same<value_type, emplace_tag> is_same_value_emplace_tag;
    typedef boost::is_same<exceptional_type, exceptional_tag> is_same_exceptional_exceptional_tag;
    typedef boost::is_same<exceptional_type, emplace_tag> is_same_exceptional_emplace_tag;
    BOOST_STATIC_ASSERT_MSG( !is_same_value_exceptional_tag::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_value_emplace_tag::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_exceptional_exceptional_tag::value, "bad ExceptionalType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_exceptional_emplace_tag::value, "bad ExceptionalType" );

    // C++03 movable support
    BOOST_COPYABLE_AND_MOVABLE(this_type)

  private:
    union {
      exceptional_type error;
      value_type value; 
    };
    bool has_value;

  public:

    // Constructors/Destructors/Assignments

    BOOST_CONSTEXPR expected(const value_type& rhs) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value(rhs)))
    : value(rhs)
    , has_value(true)
    {}
    
    // Move constructor from a value
    BOOST_CONSTEXPR expected(BOOST_RV_REF(value_type) rhs)
    : value(boost::move(rhs))
    , has_value(true)
    {}

    expected(const expected& rhs) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value(rhs.value)))
    : has_value(rhs.has_value)
    {
      if (has_value)
      {
        ::new (&value) value_type(rhs.value);
      }
      else
      {
        ::new (&error) exceptional_type(rhs.error);
      }
    }

    expected(BOOST_RV_REF(expected) rhs) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(boost::move(rhs.value)))
    : has_value(rhs.has_value)
    {
      if (has_value)
      {
        ::new (&value) value_type(boost::move(rhs.value));
      }
      else
      {
        ::new (&error) exceptional_type(boost::move(rhs.error));
      }
    }

    expected(exceptional_tag, exceptional_type const& e) // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(error(e)))
    : error(e)
    , has_value(false)
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    BOOST_CONSTEXPR expected(exceptional_tag, E const& e)
    : error(traits_type::make_exceptional(e))
    , has_value(false)
    {}

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <class... Args>
    BOOST_CONSTEXPR explicit expected(emplace_tag, Args&&... args)
    : value(boost::forward<Args>(args)...)
    , has_value(true)
    {}
#endif

    expected() // BOOST_NOEXCEPT
    : error(traits_type::catch_exception())
    , has_value(false)
    {}

    ~expected() // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value.~value_type()))
    {
      if (valid()) value.~value_type();
      else error.~exceptional_type();
    }

    /// Modifiers
    void swap(expected& rhs) // BOOST_NOEXCEPT_IF( ... )
    {
      if (has_value)
      {
        if (rhs.has_value)
        {
          boost::swap(value, rhs.value);
        }
        else
        {
          exceptional_type t = boost::move(rhs.error);
          new (&rhs.value) value_type(boost::move(value));
          new (&error) exceptional_type(t);
          boost::swap(has_value, rhs.has_value);
        }
      }
      else
      {
        if (rhs.has_value)
        {
          rhs.swap(*this);
        }
        else
        {
          boost::swap(error, rhs.error);
        }
      }
    }

    // Observers
    bool valid() const BOOST_NOEXCEPT
    {
      return has_value;
    }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    explicit operator bool() const noexcept
    {
      return valid();
    }
#endif

    const value_type& get() const
    {
      if (!valid()) traits_type::bad_access(error);
      return value;
    }

    value_type& get()
    {
      if (!valid()) traits_type::bad_access(error);
      return value;
    }

    /// factories
    /*template <typename F>
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
    }*/
  };
/*
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
  */
}

#endif // BOOST_EXPECTED_HPP