// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013 Vicente J. Botet Escriba
// (C) Copyright 2013 Pierre Talbot

#ifndef BOOST_EXPECTED_HPP
#define BOOST_EXPECTED_HPP

#define XXX

#include <stdexcept>

#include <boost/config.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/move/move.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <type_traits>
#include <utility>

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

// workaround: std utility functions aren't constexpr yet
template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type& t) noexcept
{
  return static_cast<T&&>(t);
}

template <class T> inline constexpr T&& constexpr_forward(typename std::remove_reference<T>::type&& t) noexcept
{
    static_assert(!std::is_lvalue_reference<T>::value, "!!");
    return static_cast<T&&>(t);
}

template <class T> inline constexpr typename std::remove_reference<T>::type&& constexpr_move(T&& t) noexcept
{
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

template<class _Ty> inline constexpr _Ty * constexpr_addressof(_Ty& _Val)
{
    return ((_Ty *) &(char&)_Val);
}

namespace detail {

  template <typename T, typename E>
  union constexpr_expected_storage {
    typedef T value_type;
    typedef E error_type;

    error_type err;
    value_type val;

    BOOST_CONSTEXPR constexpr_expected_storage()
      BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
    : val()
    {}

    BOOST_CONSTEXPR constexpr_expected_storage(exceptional_t, error_type const& e)
    : err(e)
    {}

    template <class... Args>
    BOOST_CONSTEXPR constexpr_expected_storage(Args&&... args)
    : val(constexpr_forward<Args>(args)...)
    {}

    ~constexpr_expected_storage() = default;
  };

  template <typename T, typename E>
  union no_constexpr_expected_storage {
    typedef T value_type;
    typedef E error_type;

    error_type err;
    value_type val;

    BOOST_CONSTEXPR no_constexpr_expected_storage()
      : val()
    {}

    BOOST_CONSTEXPR no_constexpr_expected_storage(exceptional_t, error_type const& e)
    : err(e)
    {}

    template <class... Args>
      BOOST_CONSTEXPR no_constexpr_expected_storage(Args&&... args) //BOOST_NOEXCEPT_IF()
      : val(constexpr_forward<Args>(args)...)
    {}

   ~no_constexpr_expected_storage() {};
  };

  BOOST_CONSTEXPR struct only_set_valid_t{} only_set_valid{};

  template <typename T, typename E>
  struct constexpr_expected_base {
    typedef T value_type;
    typedef E error_type;

    bool has_value;
    constexpr_expected_storage<T, E> storage;

    BOOST_CONSTEXPR constexpr_expected_base()
      BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
    : has_value(true), storage() {}

    BOOST_CONSTEXPR constexpr_expected_base(only_set_valid_t, bool has_value)
    : has_value(has_value) {}

    BOOST_CONSTEXPR constexpr_expected_base(const value_type& v)
    : has_value(true), storage(v) {}

    BOOST_CONSTEXPR constexpr_expected_base(value_type&& v)
    : has_value(true), storage(constexpr_move(v)) {}

    BOOST_CONSTEXPR constexpr_expected_base(exceptional_t, error_type const& e)
    : has_value(false), storage(exceptional, e)
    {}

    template <class... Args>
    explicit BOOST_CONSTEXPR
    constexpr_expected_base(in_place_t, Args&&... args)
    : has_value(true), storage(constexpr_forward<Args>(args)...) {}

     ~constexpr_expected_base() = default;
  };

  template <typename T, typename E>
  struct no_constexpr_expected_base {
    typedef T value_type;
    typedef E error_type;

    bool has_value;
    no_constexpr_expected_storage<T, E> storage;

    BOOST_CONSTEXPR no_constexpr_expected_base()
      BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
    : has_value(true), storage() {}

    BOOST_CONSTEXPR no_constexpr_expected_base(only_set_valid_t, bool has_value)
    : has_value(has_value) {}

    BOOST_CONSTEXPR no_constexpr_expected_base(const value_type& v)
    : has_value(true), storage(v) {}

    BOOST_CONSTEXPR no_constexpr_expected_base(value_type&& v)
    : has_value(true), storage(constexpr_move(v)) {}

    BOOST_CONSTEXPR no_constexpr_expected_base(exceptional_t, error_type const& e)
    : has_value(false), storage(exceptional, e)
    {}

    template <class... Args>
    explicit BOOST_CONSTEXPR
    no_constexpr_expected_base(in_place_t, Args&&... args)
    : has_value(true), storage(constexpr_forward<Args>(args)...) {}

    ~no_constexpr_expected_base() {
      if (has_value) storage.val.~value_type();
      else storage.err.~error_type();
    }
  };

  template <typename T, typename E>
    using expected_base = typename std::conditional<
      has_trivial_destructor<T>::value && has_trivial_destructor<E>::value,
      constexpr_expected_base<T,E>,
      no_constexpr_expected_base<T,E>
    >::type;
}

  template <typename ValueType, typename ErrorType>
  class expected
#if defined XXX
  : detail::expected_base<ValueType, ErrorType>
#endif
  {
  public:
    typedef ValueType value_type;
    typedef ErrorType error_type;
    typedef expected_traits<error_type> traits_type;

  private:
    typedef expected<value_type, error_type> this_type;
#if defined XXX
    typedef detail::expected_base<value_type, error_type> base_type;
#endif

    // Static asserts.
    typedef boost::is_same<value_type, exceptional_t> is_same_value_exceptional_t;
    typedef boost::is_same<value_type, in_place_t> is_same_value_in_place_t;
    typedef boost::is_same<error_type, exceptional_t> is_same_error_exceptional_t;
    typedef boost::is_same<error_type, in_place_t> is_same_error_in_place_t;
    BOOST_STATIC_ASSERT_MSG( !is_same_value_exceptional_t::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_value_in_place_t::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_error_exceptional_t::value, "bad ErrorType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );

#if defined XXX
    value_type* dataptr() { return std::addressof(base_type::storage.val); }
    BOOST_CONSTEXPR const value_type* dataptr() const { return static_addressof(base_type::storage.val); }
    error_type* errorptr() { return std::addressof(base_type::storage.err); }
    BOOST_CONSTEXPR const error_type* errorptr() const { return static_addressof(base_type::storage.err); }

#if ! defined BOOST_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
    BOOST_CONSTEXPR const bool& contained_has_value() const& { return base_type::has_value; }
    bool& contained_has_value() & { return base_type::has_value; }
    bool&& contained_has_value() && { return std::move(base_type::has_value); }


    BOOST_CONSTEXPR const value_type& contained_val() const& { return base_type::storage.val; }
    value_type& contained_val() & { return base_type::storage.val; }
    value_type&& contained_val() && { return std::move(base_type::storage.val); }

    BOOST_CONSTEXPR const error_type& contained_err() const& { return base_type::storage.err; }
    error_type& contained_err() & { return base_type::storage.err; }
    error_type&& contained_err() && { return std::move(base_type::storage.err); }

#else
    BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
    bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
    BOOST_CONSTEXPR const value_type& contained_val() const { return base_type::storage.val; }
    value_type& contained_val() { return base_type::storage.val; }
    BOOST_CONSTEXPR const error_type& contained_err() const { return base_type::storage.err; }
    error_type& contained_err() { return base_type::storage.err; }
#endif
#else

    value_type* dataptr() { return std::addressof(val); }
    BOOST_CONSTEXPR const value_type* dataptr() const { return static_addressof(val); }
    error_type* errorptr() { return std::addressof(err); }
    BOOST_CONSTEXPR const error_type* errorptr() const { return static_addressof(err); }

    BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return has_value; }
    bool& contained_has_value() BOOST_NOEXCEPT { return has_value; }
    BOOST_CONSTEXPR const value_type& contained_val() const { return val; }
    value_type& contained_val() { return val; }
    BOOST_CONSTEXPR const error_type& contained_err() const { return err; }
    error_type& contained_err() { return err; }


#endif

    // C++03 movable support
    BOOST_COPYABLE_AND_MOVABLE(this_type)

#if ! defined XXX
 private:
    union {
      error_type err;
      value_type val;
    };
    bool has_value;
#endif

  public:

    // About noexcept specification:
    //  has_nothrow_move_constructor is not yet into Boost.TypeTraits.

    // Constructors/Destructors/Assignments

    //../../../boost/expected/expected.hpp:140:91: error: called object type 'value_type' (aka 'long') is not a function or function pointer
    //BOOST_CONSTEXPR expected(const value_type& rhs) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(val(rhs)))


    BOOST_CONSTEXPR expected(const value_type& v
      , REQUIRES(std::is_copy_constructible<value_type>::value)
    )
    // BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(val(v)))
#if  defined XXX
    : base_type(v)
    {}
#else
    : val(v)
    , has_value(true)
    {}
#endif

    BOOST_CONSTEXPR expected(BOOST_RV_REF(value_type) v
      , REQUIRES(std::is_move_constructible<value_type>::value)
    )
    //BOOST_NOEXCEPT_IF(
    //      std::has_nothrow_move_constructor<value_type>::value
    //  &&  std::has_nothrow_move_constructor<error_type>::value
    //)
#if  defined XXX
    : base_type(constexpr_move(v))
#else
    : val(boost::move(v))
    , has_value(true)
#endif
    {}

    expected(const expected& rhs
      , REQUIRES( std::is_copy_constructible<value_type>::value
             && std::is_copy_constructible<error_type>::value)
    )
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<value_type>::value &&
      has_nothrow_copy_constructor<error_type>::value
    )
#if  defined XXX
    : base_type(detail::only_set_valid, rhs.valid())
#else
    : has_value(rhs.valid())
#endif
    {
      if (rhs.valid())
      {
        ::new (dataptr()) value_type(rhs.contained_val());
      }
      else
      {
        ::new (errorptr()) error_type(rhs.contained_err());
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
#if  defined XXX
    : base_type(detail::only_set_valid, rhs.valid())
#else
    : has_value(rhs.has_value)
#endif
    {
      if (rhs.valid())
      {
        ::new (dataptr()) value_type(boost::move(rhs.contained_val()));
      }
      else
      {
        ::new (errorptr()) error_type(boost::move(rhs.contained_err()));
      }
    }

    expected(exceptional_t, error_type const& e
      , REQUIRES(std::is_copy_constructible<error_type>::value)

    )
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<error_type>::value
    )
#if  defined XXX
    : base_type(exceptional, e)
#else
    : err(e)
    , has_value(false)
#endif
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    expected(exceptional_t, E const& e)
#if  defined XXX
    : base_type(exceptional, traits_type::from_error(e))
#else
    : err(traits_type::from_error(e))
    , has_value(false)
#endif
    {}

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <class... Args
      //, REQUIRES(std::is_constructible<value_type, Args&...>::value)
    >
    BOOST_CONSTEXPR explicit expected(in_place_t, Args&&... args)
#if  defined XXX
    : base_type(boost::forward<Args>(args)...)
#else
    : val(boost::forward<Args>(args)...)
    , has_value(true)
#endif
    {}
#endif

    BOOST_CONSTEXPR expected(
       REQUIRES(std::is_default_constructible<value_type>::value)
    ) BOOST_NOEXCEPT_IF(
      has_nothrow_default_constructor<value_type>::value
    )
#if  defined XXX
    : base_type()
#else
    : val()
    , has_value(true)
#endif
    {
    }

    expected(exceptional_t)
#if  defined XXX
    : base_type(exceptional, traits_type::default_error())
#else
    : err(traits_type::default_error())
    , has_value(false)
#endif
    {}


#if  defined XXX
    ~expected() = default;
#else
    ~expected()
    //BOOST_NOEXCEPT_IF(
    //  is_nothrow_destructible<value_type>::value &&
    //  is_nothrow_destructible<error_type>::value
    //)
    {
      if (valid()) val.~value_type();
      else err.~error_type();
    }
#endif

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
      if (valid())
      {
        if (rhs.valid())
        {
          boost::swap(contained_val(), rhs.contained_val());
        }
        else
        {
          error_type t = boost::move(rhs.contained_err());
          new (rhs.dataptr()) value_type(boost::move(contained_val()));
          new (errorptr()) error_type(t);
          std::swap(contained_has_value(), rhs.contained_has_value());
        }
      }
      else
      {
        if (rhs.valid())
        {
          rhs.swap(*this);
        }
        else
        {
          boost::swap(contained_err(), rhs.contained_err());
        }
      }
    }

    // Observers
    BOOST_CONSTEXPR bool valid() const BOOST_NOEXCEPT
    {
      return contained_has_value();
    }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
    explicit operator bool() const BOOST_NOEXCEPT
    {
      return valid();
    }
#endif

    const value_type& get() const
    {
      if (!valid()) traits_type::bad_access(contained_err());
      return contained_val();
    }

    value_type& get()
    {
      if (!valid()) traits_type::bad_access(contained_err());
      return contained_val();
    }

    BOOST_CONSTEXPR value_type const& operator*() const BOOST_NOEXCEPT
    {
      return contained_val();
    }

    value_type& operator*() BOOST_NOEXCEPT
    {
      return contained_val();
    }
    BOOST_CONSTEXPR value_type const* operator->() const BOOST_NOEXCEPT
    {
      return dataptr();
    }

    value_type* operator->() BOOST_NOEXCEPT
    {
      return dataptr();
    }

    error_type error() const
    {
      return contained_err();
    }

    // Utilities

#if ! defined BOOST_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

    template <class V>
    BOOST_CONSTEXPR value_type value_or(V&& v) const&   {
      return *this ? **this : static_cast<value_type>(constexpr_forward<V>(v));
    }

    template <class V>
    BOOST_CONSTEXPR value_type value_or(V&& v) const &&  {
      return *this ? std::move(const_cast<expected<value_type, error_type>&>(*this).contained_val()) : static_cast<value_type>(constexpr_forward<V>(v));
    }

# else

    template <class V>
    BOOST_CONSTEXPR value_type value_or(V&& v) const
    {
      return *this ? **this : static_cast<value_type>(constexpr_forward<V>(v));
    }

# endif

    template <typename F>
    expected<typename boost::result_of<F(expected)>::type, ErrorType>
    then(F fuct)
    {
      return make_expected(funct(contained_val()));
    }

    template <typename F>
    expected<typename boost::result_of<F(value_type)>::type, ErrorType>
    next(F fuct)
    {
      typedef typename boost::result_of<F(value_type)>::type result_value_type;
      if (valid()) {
        return make_expected<ErrorType>(funct(contained_val()));
      } else {
        return make_expected_from_error<result_value_type>(contained_err());
      }
    }
    template <typename F>
    expected
    recover(F fuct)
    {
      if (valid()) {
        return funct(contained_val());
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

