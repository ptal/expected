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
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <type_traits>

# define REQUIRES(...) typename std::enable_if<__VA_ARGS__, void*>::type = 0
# define RETURN_IF(RET, ...) typename std::enable_if<__VA_ARGS__, RET>::type



# if defined __clang__
# if (__clang_major__ < 2) || (__clang_major__ == 2) && (__clang_minor__ < 9)
# define BOOST_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
# endif
# else
# define BOOST_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
# endif

namespace boost
{


  // bad_expected_access exception class.
  template <class Error>
  class bad_expected_access : public std::logic_error
  {
    public:
      typedef Error error_type;
    private:
      error_type error_value;
    public:
      bad_expected_access(const Error& e)
      : std::logic_error("Found an error instead of the expected value.")
      , error_value(e)
      {}

      error_type& error() { return error_value; }
      const error_type& error() const { return error_value; }

      // Add implicit/explicit conversion to error_type ?
  };

  // Traits classes
  template <typename ErrorType>
  struct expected_traits
  {
    typedef ErrorType error_type;

    template <class E>
    static error_type from_error(E const& e)
    {
      return error_type(e);
    }

    static error_type default_error()
    {
      throw;
    }

    static void bad_access(const error_type &e)
    {
      boost::throw_exception(bad_expected_access<error_type>(e));
    }
  };

  // Specialization for exception_ptr
  template <>
  struct expected_traits<boost::exception_ptr>
  {
    typedef boost::exception_ptr error_type;

    template <class E>
    static error_type from_error(E const& e)
    {
      return boost::copy_exception(e);
    }

    static error_type default_error()
    {
      return boost::current_exception();
    }

    static void bad_access(const error_type &e)
    {
      boost::rethrow_exception(e);
    }
  };

#if ! defined BOOST_NO_CXX11_HDR_EXCEPTION  && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
  template <>
  struct expected_traits<std::exception_ptr>
  : public expected_traits<boost::exception_ptr>
  {
    typedef std::exception_ptr error_type;
  };
#endif

  struct exceptional_t {};
  BOOST_CONSTEXPR_OR_CONST exceptional_t exceptional = {};

  struct in_place_t {};
  BOOST_CONSTEXPR_OR_CONST in_place_t in_place2 = {};

  template <typename ValueType, typename ErrorType=boost::exception_ptr>
  class expected;

    // Factories
#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
  template<typename T, typename E, typename... Args>
  expected<T,E> make_expected(Args&&... args);

  template<typename T, typename... Args>
  expected<T> make_expected(Args&&... args);
#endif

  template <typename T>
  expected<T> make_expected_from_error() BOOST_NOEXCEPT;

  template <typename T, typename E>
  expected<T, E> make_expected_from_error();

  template <typename T, typename U, typename E>
  typename boost::disable_if<
    boost::is_same<U, E>,
    expected<T,U>
  >::type make_expected_from_error(E const& e);

  // Requires  typeid(e) == typeid(E)
  template <typename T, typename E>
    RETURN_IF(expected<T>, boost::is_base_of<std::exception, E>::value || boost::is_base_of<boost::exception, E>::value)
  make_expected_from_error(E const& e) BOOST_NOEXCEPT;

  template <typename T, typename E>
  typename boost::disable_if_c<
    boost::is_base_of<std::exception, E>::value || boost::is_base_of<boost::exception, E>::value,
    expected<T,E>
  >::type make_expected_from_error(E const& e);

  template <typename F>
  expected<typename boost::result_of<F()>::type>
  make_expected_from_call(F fuct) BOOST_NOEXCEPT
  {
    try {
      return make_expected(fuct());
    } catch (...) {
      return make_expected_from_error<typename boost::result_of<F()>::type>();
    }
  }


  template <typename ValueType, typename ErrorType>
  class expected
  {
  public:
    typedef ValueType value_type;
    typedef ErrorType error_type;
    typedef expected_traits<error_type> traits_type;

  private:
    typedef expected<value_type, error_type> this_type;

    // Static asserts.
    typedef boost::is_same<value_type, exceptional_t> is_same_value_exceptional_t;
    typedef boost::is_same<value_type, in_place_t> is_same_value_in_place_t;
    typedef boost::is_same<error_type, exceptional_t> is_same_error_exceptional_t;
    typedef boost::is_same<error_type, in_place_t> is_same_error_in_place_t;
    BOOST_STATIC_ASSERT_MSG( !is_same_value_exceptional_t::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_value_in_place_t::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_error_exceptional_t::value, "bad ErrorType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );

    // C++03 movable support
    BOOST_COPYABLE_AND_MOVABLE(this_type)

  private:
    union {
      error_type err;
      value_type val;
    };
    bool has_value;

  public:

    // About noexcept specification:
    //  has_nothrow_move_constructor is not yet into Boost.TypeTraits.

    // Constructors/Destructors/Assignments

    //../../../boost/expected/expected.hpp:140:91: error: called object type 'value_type' (aka 'long') is not a function or function pointer
    //BOOST_CONSTEXPR expected(const value_type& rhs) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(val(rhs)))


    BOOST_CONSTEXPR expected(const value_type& rhs
      , REQUIRES(std::is_copy_constructible<value_type>::value)
    )
    // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(val(rhs)))
    : val(rhs)
    , has_value(true)
    {}

    BOOST_CONSTEXPR expected(BOOST_RV_REF(value_type) rhs
      , REQUIRES(std::is_move_constructible<value_type>::value)
    )
    //BOOST_NOEXCEPT_IF(
    //      std::has_nothrow_move_constructor<value_type>::value
    //  &&  std::has_nothrow_move_constructor<error_type>::value
    //)
    : val(boost::move(rhs))
    , has_value(true)
    {}

    expected(const expected& rhs
      , REQUIRES( std::is_copy_constructible<value_type>::value
             && std::is_copy_constructible<error_type>::value)
    )
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<value_type>::value &&
      has_nothrow_copy_constructor<error_type>::value
    )
    : has_value(rhs.has_value)
    {
      if (has_value)
      {
        ::new (&val) value_type(rhs.val);
      }
      else
      {
        ::new (&err) error_type(rhs.err);
      }
    }

    expected(BOOST_RV_REF(expected) rhs
      , REQUIRES( std::is_move_constructible<value_type>::value
             && std::is_move_constructible<error_type>::value)
    )
    //BOOST_NOEXCEPT_IF(
    //  std::has_nothrow_move_constructor<value_type>::value &&
    //  std::has_nothrow_move_constructor<error_type>::value
    //)
    : has_value(rhs.has_value)
    {
      if (has_value)
      {
        ::new (&val) value_type(boost::move(rhs.val));
      }
      else
      {
        ::new (&err) error_type(boost::move(rhs.err));
      }
    }

    expected(exceptional_t, error_type const& e
      , REQUIRES(std::is_copy_constructible<error_type>::value)

    )
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<error_type>::value
    )
    : err(e)
    , has_value(false)
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    expected(exceptional_t, E const& e)
    : err(traits_type::from_error(e))
    , has_value(false)
    {}

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <class... Args
      //, REQUIRES(std::is_constructible<value_type, Args&...>::value)
    >
    BOOST_CONSTEXPR explicit expected(in_place_t, Args&&... args)
    : val(boost::forward<Args>(args)...)
    , has_value(true)
    {}
#endif

    expected(
       REQUIRES(std::is_default_constructible<value_type>::value)
    ) BOOST_NOEXCEPT_IF(
      has_nothrow_default_constructor<value_type>::value
    )
    : val()
    , has_value(true)
    {
    }

    expected(exceptional_t)
    : err(traits_type::default_error())
    , has_value(false)
    {}


    ~expected()
    //BOOST_NOEXCEPT_IF(
    //  is_nothrow_destructible<value_type>::value &&
    //  is_nothrow_destructible<error_type>::value
    //)
    {
      if (valid()) val.~value_type();
      else err.~error_type();
    }

    // Assignments
    expected& operator=(BOOST_COPY_ASSIGN_REF(expected) e)
    {
      this_type(e).swap(*this);
      return *this;
    }

    expected& operator=(BOOST_RV_REF(expected) e)
    {
      this_type(boost::move(e)).swap(*this);
      return *this;
    }

    expected& operator=(BOOST_COPY_ASSIGN_REF(value_type) value)
    {
      this_type(value).swap(*this);
      return *this;
    }

    expected& operator=(BOOST_RV_REF(value_type) value)
    {
      this_type(boost::move(value)).swap(*this);
      return *this;
    }

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <class... Args>
    expected& emplace(Args&&... args)
    {
      // Why emplace doesn't work (instead of in_place_t()) ?
      this_type(in_place_t(), boost::forward<Args>(args)...).swap(*this);
      return *this;
    }
#endif

    // Modifiers
    void swap(expected& rhs)
    {
      if (has_value)
      {
        if (rhs.has_value)
        {
          boost::swap(val, rhs.val);
        }
        else
        {
          error_type t = boost::move(rhs.err);
          new (&rhs.val) value_type(boost::move(val));
          new (&err) error_type(t);
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
          boost::swap(err, rhs.err);
        }
      }
    }

    // Observers
    bool valid() const BOOST_NOEXCEPT
    {
      return has_value;
    }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    explicit operator bool() const BOOST_NOEXCEPT
    {
      return valid();
    }
#endif

    const value_type& get() const
    {
      if (!valid()) traits_type::bad_access(err);
      return val;
    }

    value_type& get()
    {
      if (!valid()) traits_type::bad_access(err);
      return val;
    }

    BOOST_CONSTEXPR value_type const& operator*() const BOOST_NOEXCEPT
    {
      return val;
    }

    value_type& operator*() BOOST_NOEXCEPT
    {
      return val;
    }
    BOOST_CONSTEXPR value_type const* operator->() const BOOST_NOEXCEPT
    {
      return &val;
    }

    value_type* operator->() BOOST_NOEXCEPT
    {
      return &val;
    }

    error_type error() const
    {
      return err;
    }

    // Utilities

#if ! defined BOOST_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

    template <class V>
    BOOST_CONSTEXPR value_type value_or(V&& v) const&   {
      return *this ? **this : static_cast<value_type>(std::forward<V>(v));
    }

    template <class V>
    BOOST_CONSTEXPR value_type value_or(V&& v) const &&  {
      return *this ? std::move(const_cast<expected<value_type, error_type>&>(*this).contained_val()) : static_cast<value_type>(std::forward<V>(v));
    }

# else

    template <class V>
    BOOST_CONSTEXPR value_type value_or(V&& v) const
    {
      return *this ? **this : static_cast<value_type>(std::forward<V>(v));
    }

# endif

    template <typename F>
    expected<typename boost::result_of<F(expected)>::type, ErrorType>
    then(F fuct)
    {
      return make_expected(funct(val));
    }

    template <typename F>
    expected<typename boost::result_of<F(value_type)>::type, ErrorType>
    next(F fuct)
    {
      typedef typename boost::result_of<F(value_type)>::type result_value_type;
      if (valid()) {
        return make_expected<ErrorType>(funct(val));
      } else {
        return make_expected_from_error<result_value_type>(err);
      }
    }
    template <typename F>
    expected
    recover(F fuct)
    {
      if (valid()) {
        return funct(val);
      } else {
        return *this;
      }
    }
  };

  // Relational operators
  template <class T, class E>
  BOOST_CONSTEXPR bool operator==(const expected<T, E>& x, const expected<T, E>& y)
  {
    return (x && y)
      ? *x == *y
      : (!x && !y)
        ?  x.error() == y.error()
        : false;

  }

  template <class T, class E>
  BOOST_CONSTEXPR bool operator!=(const expected<T, E>& x, const expected<T, E>& y)
  {
    return !(x == y);
  }

  template <class T, class E>
  BOOST_CONSTEXPR bool operator<(const expected<T, E>& x, const expected<T, E>& y)
  {
    return (x)
      ? (y) ? *x < *y : true
      : (y) ? false : x.error() < y.error();
  }

  template <class T, class E>
  BOOST_CONSTEXPR bool operator>(const expected<T, E>& x, const expected<T, E>& y)
  {
    return (y < x);
  }

  template <class T, class E>
  BOOST_CONSTEXPR bool operator<=(const expected<T, E>& x, const expected<T, E>& y)
  {
    return !(y < x);
  }

  template <class T, class E>
  BOOST_CONSTEXPR bool operator>=(const expected<T, E>& x, const expected<T, E>& y)
  {
    return !(x < y);
  }

  // Specialized algorithms
  template <class T>
  void swap(expected<T>& x, expected<T>& y) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(x.swap(y)))
  {
    x.swap(y);
  }

  // Factories
#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
  template<typename T, typename E, typename... Args>
  expected<T,E> make_expected(Args&&... args)
  {
    return expected<T,E>(in_place2, boost::forward<Args>(args)...);
  }

  template<typename T, typename... Args>
  expected<T> make_expected(Args&&... args)
  {
    return expected<T>(in_place2, boost::forward<Args>(args)...);
  }
#endif

  template <typename T>
  expected<T> make_expected_from_error() BOOST_NOEXCEPT
  {
    return expected<T>(exceptional);
  }

  template <typename T, typename E>
  expected<T, E> make_expected_from_error()
  {
    return expected<T, E>(exceptional);
  }

  template <typename T, typename U, typename E>
  typename boost::disable_if<
    boost::is_same<U, E>,
    expected<T,U>
  >::type make_expected_from_error(E const& e)
  {
    return expected<T, U>(exceptional, e);
  }

  // Requires  typeid(e) == typeid(E)
  template <typename T, typename E>
    RETURN_IF(expected<T>, boost::is_base_of<std::exception, E>::value || boost::is_base_of<boost::exception, E>::value)
  make_expected_from_error(E const& e) BOOST_NOEXCEPT
  {
    return expected<T>(exceptional, e);
  }

  template <typename T, typename E>
  typename boost::disable_if_c<
    boost::is_base_of<std::exception, E>::value || boost::is_base_of<boost::exception, E>::value,
    expected<T,E>
  >::type make_expected_from_error(E const& e)
  {
    return expected<T, E>(exceptional, e);
  }

/*
  template <typename F>
  expected<typename boost::result_of<F()>::type>
  make_expected_from_call(F&& fuct) BOOST_NOEXCEPT
  {
    try {
      return make_expected(fun());
    } catch (...) {
      return make_expected_from_error();
    }
  }
  */
}

#endif // BOOST_EXPECTED_HPP

