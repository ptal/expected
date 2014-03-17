// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013,2014 Vicente J. Botet Escriba
// (C) Copyright 2013 Pierre Talbot

#ifndef BOOST_EXPECTED_HPP
#define BOOST_EXPECTED_HPP

#include <boost/functional/adaptor.hpp>
#include <boost/expected/unexpected.hpp>

#include <boost/config.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/utility/swap.hpp>

#include <stdexcept>
#include <utility>
#include <initializer_list>


#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
#if defined __clang__
#if ! __has_feature(cxx_relaxed_constexpr)
#define BOOST_NO_CXX14_RELAXED_CONSTEXPR
#endif
#else
#define BOOST_NO_CXX14_RELAXED_CONSTEXPR
#endif
#endif


// TODO: We'd need to check if std::is_default_constructible is there too.
#ifndef BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  #define BOOST_EXPECTED_USE_DEFAULT_CONSTRUCTOR
  #include <type_traits>
#endif

#if defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
  #include <boost/preprocessor/facilities/intercept.hpp>
  #include <boost/preprocessor/repetition/enum_params.hpp>
  #include <boost/preprocessor/repetition/repeat_from_to.hpp>
  #ifndef BOOST_EXPECTED_EMPLACE_MAX_ARGS
    #define BOOST_EXPECTED_EMPLACE_MAX_ARGS 10
  #endif

  #define MAKE_BOOST_FWD_REF_ARG(z, count, unused) BOOST_PP_COMMA_IF(count) BOOST_FWD_REF(Arg##count) arg##count
  #define MAKE_BOOST_FWD_PARAM(z, count, unused) BOOST_PP_COMMA_IF(count) boost::forward<Arg##count>(arg##count)
#endif

# define REQUIRES(...) typename ::boost::enable_if_c<__VA_ARGS__, void*>::type = 0
# define T_REQUIRES(...) typename = typename ::boost::enable_if_c<(__VA_ARGS__)>::type

# if defined __clang__
#  if (__clang_major__ < 2) || (__clang_major__ == 2) && (__clang_minor__ < 9)
#   define BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
#  endif
# elif defined __GNUC__
#  if (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__ < 40801) || !defined(__GXX_EXPERIMENTAL_CXX0X__)
#   define BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
#  endif
# else
#  define BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
# endif


// ../../../boost/expected/expected.hpp: In instantiation of ‘class boost::expected<int>’:
// test_expected.cpp:79:17:   required from here
// ../../../boost/expected/expected.hpp:596:15: desole, pas implante: use of ‘type_pack_expansion’ in template

#if defined BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
# define BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#else
# if defined __GNUC__
#  if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 8) || !defined(__GXX_EXPERIMENTAL_CXX0X__)
#   define BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#  endif
# endif
#endif


namespace boost {
namespace detail {

template <typename T>
struct has_overloaded_addressof
{
  template <class X>
  static BOOST_CONSTEXPR bool has_overload(...) { return false; }

  template <class X, size_t S = sizeof(std::declval< X&>().operator&()) >
  static BOOST_CONSTEXPR bool has_overload(bool) { return true; }

  BOOST_CONSTEXPR static bool value = has_overload<T>(true);
};

template <typename T>
BOOST_CONSTEXPR T* static_addressof(T& ref,
  REQUIRES(!has_overloaded_addressof<T>::value))
{
  return &ref;
}

template <typename T>
BOOST_CONSTEXPR T* static_addressof(T& ref,
  REQUIRES(has_overloaded_addressof<T>::value))
{
  return std::addressof(ref);
}

} // namespace detail

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
template <typename ErrorType, class Exception>
struct expected_error_traits
{
  typedef ErrorType error_type;
  typedef Exception exception_type;

  template <class E>
  static error_type from_error(error_type const& e)
  {
    return error_type(e);
  }

  static void bad_access(const error_type &e)
  {
    throw Exception(e);
  }
};

template <typename ErrorType>
struct expected_traits : expected_error_traits<ErrorType, bad_expected_access<ErrorType> >
{
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

  static void bad_access(const error_type &e)
  {
    boost::rethrow_exception(e);
  }
};

#if ! defined BOOST_NO_CXX11_HDR_EXCEPTION  && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
template <>
struct expected_traits<std::exception_ptr>
{
  typedef std::exception_ptr error_type;

  template <class E>
  static error_type from_error(E const& e)
  {
    return std::make_exception_ptr(e);
  }

  static void bad_access(const error_type &e)
  {
    std::rethrow_exception(e);
  }
};
#endif

struct in_place_t {};
BOOST_CONSTEXPR_OR_CONST in_place_t in_place2 = {};

// workaround: std utility functions aren't constexpr yet
template <class T> inline
BOOST_CONSTEXPR T&& constexpr_forward(typename std::remove_reference<T>::type& t) BOOST_NOEXCEPT
{
  return static_cast<T&&>(t);
}

template <class T> inline
BOOST_CONSTEXPR T&& constexpr_forward(typename std::remove_reference<T>::type&& t) BOOST_NOEXCEPT
{
    static_assert(!std::is_lvalue_reference<T>::value, "!!");
    return static_cast<T&&>(t);
}

template <class T> inline
BOOST_CONSTEXPR typename std::remove_reference<T>::type&& constexpr_move(T&& t) BOOST_NOEXCEPT
{
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}

template<class T> inline
BOOST_CONSTEXPR T * constexpr_addressof(T& Val)
{
  return ((T *) &(char&)Val);
}

namespace detail {

template <typename T, typename E>
union trivial_expected_storage
{
  typedef T value_type;
  typedef E error_type;
  typedef expected_traits<error_type> traits_type;

  error_type err;
  value_type val;

  BOOST_CONSTEXPR trivial_expected_storage()
    BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
  : val()
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_type> const& e)
  : err(e.value())
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  template <class... Args>
  BOOST_CONSTEXPR trivial_expected_storage(BOOST_FWD_REF(Args)... args)
  : val(constexpr_forward<Args>(args)...)
  {}

  ~trivial_expected_storage() = default;
};

template <typename E>
union trivial_expected_storage<void, E>
{
  typedef E error_type;
  typedef expected_traits<error_type> traits_type;

  error_type err;
  unsigned char dummy;

  BOOST_CONSTEXPR trivial_expected_storage()
  : dummy(0)
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_type> const& e)
  : err(e.value())
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  ~trivial_expected_storage() = default;
};

template <typename T, typename E>
union no_trivial_expected_storage
{
  typedef T value_type;
  typedef E error_type;
  typedef expected_traits<error_type> traits_type;

  error_type err;
  value_type val;

  BOOST_CONSTEXPR no_trivial_expected_storage()
  : val()
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_type> const& e)
  : err(e.value())
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  template <class... Args>
  BOOST_CONSTEXPR no_trivial_expected_storage(BOOST_FWD_REF(Args)... args) //BOOST_NOEXCEPT_IF()
  : val(constexpr_forward<Args>(args)...)
  {}

  ~no_trivial_expected_storage() {};
};

template <typename E>
union no_trivial_expected_storage<void, E>
{
  typedef E error_type;
  typedef expected_traits<error_type> traits_type;

  error_type err;
  unsigned char dummy;

  BOOST_CONSTEXPR no_trivial_expected_storage()
  : dummy(0)
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_type> const& e)
  : err(e.value())
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  ~no_trivial_expected_storage() {};
};

BOOST_CONSTEXPR struct only_set_valid_t{} only_set_valid{};

template <typename T, typename E>
struct trivial_expected_base
{
  typedef T value_type;
  typedef E error_type;

  bool has_value;
  trivial_expected_storage<T, E> storage;

  BOOST_CONSTEXPR trivial_expected_base()
    BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
  : has_value(true), storage()
  {}

  BOOST_CONSTEXPR trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value)
  {}

  BOOST_CONSTEXPR trivial_expected_base(const value_type& v)
  : has_value(true), storage(v)
  {}

  BOOST_CONSTEXPR trivial_expected_base(BOOST_FWD_REF(value_type) v)
  : has_value(true), storage(constexpr_move(v))
  {}

  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}

  template <class... Args>
  explicit BOOST_CONSTEXPR
  trivial_expected_base(in_place_t, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit BOOST_CONSTEXPR
  trivial_expected_base(in_place_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(il, constexpr_forward<Args>(args)...)
  {}

   ~trivial_expected_base() = default;
};

template <typename E>
struct trivial_expected_base<void, E>
{
  typedef void value_type;
  typedef E error_type;

  bool has_value;
  trivial_expected_storage<void, E> storage;

  BOOST_CONSTEXPR trivial_expected_base()
  : has_value(true), storage() {}

  BOOST_CONSTEXPR trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value) {}

  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}

   ~trivial_expected_base() = default;
};

template <typename T, typename E>
struct no_trivial_expected_base
{
  typedef T value_type;
  typedef E error_type;

  bool has_value;
  no_trivial_expected_storage<T, E> storage;

  BOOST_CONSTEXPR no_trivial_expected_base()
    BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
  : has_value(true), storage()
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(v)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(BOOST_FWD_REF(value_type) v)
  : has_value(true), storage(constexpr_move(v))
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}

  template <class... Args>
  explicit BOOST_CONSTEXPR
  no_trivial_expected_base(in_place_t, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit BOOST_CONSTEXPR
  no_trivial_expected_base(in_place_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(il, constexpr_forward<Args>(args)...)
  {}

  ~no_trivial_expected_base()
  {
    if (has_value) storage.val.~value_type();
    else storage.err.~error_type();
  }
};

template <typename E>
struct no_trivial_expected_base<void, E> {
  typedef void value_type;
  typedef E error_type;

  bool has_value;
  no_trivial_expected_storage<void, E> storage;

  BOOST_CONSTEXPR no_trivial_expected_base()
  : has_value(true), storage() {}

  BOOST_CONSTEXPR no_trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value) {}


  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}

  ~no_trivial_expected_base() {
    if (! has_value)
      storage.err.~error_type();
  }
};

template <typename T, typename E>
  using expected_base = typename std::conditional<
    has_trivial_destructor<T>::value && has_trivial_destructor<E>::value,
    trivial_expected_base<T,E>,
    no_trivial_expected_base<T,E>
  >::type;

} // namespace detail

template <typename ValueType, typename ErrorType=std::exception_ptr>
class expected;

template <typename T>
struct is_expected : false_type {};
template <typename T, typename E>
struct is_expected<expected<T,E> > : true_type {};

template <typename ValueType, typename ErrorType>
class expected
: detail::expected_base<ValueType, ErrorType>
{
public:
  typedef ValueType value_type;
  typedef ErrorType error_type;
  typedef expected_traits<error_type> traits_type;

private:
  typedef expected<value_type, error_type> this_type;
  typedef detail::expected_base<value_type, error_type> base_type;

  // Static asserts.
  //typedef boost::is_same<value_type, exceptional_t> is_same_value_exceptional_t;
  typedef boost::is_same<value_type, in_place_t> is_same_value_in_place_t;
  //typedef boost::is_same<error_type, exceptional_t> is_same_error_exceptional_t;
  typedef boost::is_same<error_type, in_place_t> is_same_error_in_place_t;
  //BOOST_STATIC_ASSERT_MSG( !is_same_value_exceptional_t::value, "bad ValueType" );
  BOOST_STATIC_ASSERT_MSG( !is_same_value_in_place_t::value, "bad ValueType" );
  //BOOST_STATIC_ASSERT_MSG( !is_same_error_exceptional_t::value, "bad ErrorType" );
  BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );

  value_type* dataptr() { return std::addressof(base_type::storage.val); }
  BOOST_CONSTEXPR const value_type* dataptr() const { return static_addressof(base_type::storage.val); }
  error_type* errorptr() { return std::addressof(base_type::storage.err); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return static_addressof(base_type::storage.err); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
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

  // C++03 movable support
  BOOST_COPYABLE_AND_MOVABLE(this_type)

public:

  template <class T>
  struct bind {
    typedef expected<T, error_type> type;
  };

  // Constructors/Destructors/Assignments

  BOOST_CONSTEXPR expected(const value_type& v
    , REQUIRES(std::is_copy_constructible<value_type>::value)
  )
  BOOST_NOEXCEPT_IF(has_nothrow_copy_constructor<value_type>::value)
  : base_type(v)
  {}

  BOOST_CONSTEXPR expected(BOOST_RV_REF(value_type) v
    , REQUIRES(std::is_move_constructible<value_type>::value)
  )
  BOOST_NOEXCEPT_IF(
        std::is_nothrow_move_constructible<value_type>::value
    &&  std::is_nothrow_move_constructible<error_type>::value
  )
  : base_type(constexpr_move(v))
  {}

  expected(const expected& rhs
    , REQUIRES( std::is_copy_constructible<value_type>::value
             && std::is_copy_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<value_type>::value &&
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(detail::only_set_valid, rhs.valid())
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
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_move_constructible<value_type>::value &&
    std::is_nothrow_move_constructible<error_type>::value
  )
  : base_type(detail::only_set_valid, rhs.valid())
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

  expected(unexpected_type<error_type> const& e
    , REQUIRES(std::is_copy_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(e)
  {}

  template <class Err>
  expected(unexpected_type<Err> const& e
//    , REQUIRES(std::is_copy_constructible<error_type>::value)
  )
//  BOOST_NOEXCEPT_IF(
//    has_nothrow_copy_constructor<error_type>::value
//  )
  : base_type(e)
  {}


#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(in_place_t, BOOST_FWD_REF(Args)... args)
  : base_type(in_place_t{}, boost::forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, std::initializer_list<U> >::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(in_place_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : base_type(in_place_t{}, il, constexpr_forward<Args>(args)...)
  {}

#endif

  BOOST_CONSTEXPR expected(
     REQUIRES(std::is_default_constructible<value_type>::value)
  ) BOOST_NOEXCEPT_IF(
    has_nothrow_default_constructor<value_type>::value
  )
  : base_type()
  {}

  ~expected() = default;

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
  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
    >
  expected& emplace(BOOST_FWD_REF(Args)... args)
    {
      // Why emplace doesn't work (instead of in_place_t()) ?
      this_type(in_place_t(), boost::forward<Args>(args)...).swap(*this);
      return *this;
    }

    template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
      //, T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
      >
    expected& emplace(std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
    {
      // Why emplace doesn't work (instead of in_place_t()) ?
      this_type(in_place_t(), il, boost::forward<Args>(args)...).swap(*this);
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
  BOOST_CONSTEXPR explicit operator bool() const BOOST_NOEXCEPT
  {
    return valid();
  }
#endif

  value_type& value()
  {
    if (!valid()) traits_type::bad_access(contained_err());
    return contained_val();
  }

  BOOST_CONSTEXPR value_type const& value() const
  {
    return valid()
      ? contained_val()
      : (
          traits_type::bad_access(contained_err()),
          contained_val()
        )
      ;
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

  BOOST_CONSTEXPR error_type const& error() const BOOST_NOEXCEPT
  {
    return contained_err();
  }

  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }


  // Utilities

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  template <class V>
  BOOST_CONSTEXPR value_type value_or(BOOST_FWD_REF(V) v) const&
  {
    return *this
      ? **this
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class V>
  BOOST_CONSTEXPR value_type value_or(BOOST_FWD_REF(V) v) const &&
  {
    return *this
      ? std::move(const_cast<expected<value_type, error_type>&>(*this).contained_val())
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class Exception>
  BOOST_CONSTEXPR value_type value_or_throw() const&
  {
    return *this
      ? **this
      : throw Exception(contained_err());
  }

  template <class Exception>
  BOOST_CONSTEXPR value_type value_or_throw() const &&
  {
    return *this
      ? std::move(const_cast<expected<value_type, error_type>&>(*this).contained_val())
      : throw Exception(contained_err());
  }

# else

  template <class V>
  BOOST_CONSTEXPR value_type value_or(BOOST_FWD_REF(V) v) const {
    return *this
      ? **this
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class Exception>
  BOOST_CONSTEXPR value_type value_or_throw() const {
    return *this
      ? **this
      : throw Exception(contained_err());
  }

# endif

  template <typename F>
  BOOST_CONSTEXPR expected<void, error_type>
  next(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(value_type)>::type, void>::value)) const
  {
    typedef expected<void, error_type> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        f(**this);
        return result_type();
    }
    return get_unexpected();
#else
    return (valid()
        ? (f(**this), result_type( ))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR expected<typename result_of<F(value_type)>::type, error_type>
  next(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(value_type)>::type, void>::value
        && !boost::is_expected<typename result_of<F(value_type)>::type>::value
        )) const
  {
    typedef expected<typename result_of<F(value_type)>::type, error_type> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        return result_type(f(**this));
    }
    return get_unexpected();
#else
    return (valid()
        ? result_type( f(value()) )
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR typename result_of<F(value_type)>::type
  next(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(value_type)>::type, void>::value
        && boost::is_expected<typename result_of<F(value_type)>::type>::value
        )
    ) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        return f(value());
    }
    return get_unexpected();
#else
     return valid()
         ? f(value())
         : typename result_of<F(value_type)>::type(get_unexpected());
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR expected<void, error_type>
  then(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(expected)>::type, void>::value)) const
  {
    typedef expected<void, error_type> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    f(boost::move(*this));
    return result_type();
#else
    return (f(boost::move(*this)), result_type());
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR expected<typename result_of<F(expected)>::type, error_type>
  then(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(expected)>::type, void>::value
        && !boost::is_expected<typename result_of<F(expected)>::type>::value
        )) const
  {
    typedef expected<typename result_of<F(value_type)>::type, error_type> result_type;
    return result_type(f(boost::move(*this)));
  }

  template <typename F>
  BOOST_CONSTEXPR typename result_of<F(expected)>::type
  then(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(expected)>::type, void>::value
        && boost::is_expected<typename result_of<F(expected)>::type>::value
        )
    ) const
  {
    return f(boost::move(*this));
  }

//  template <typename H>
//  BOOST_CONSTEXPR expected<void, error_type>
//  then(BOOST_RV_REF(adaptor_holder<H>) f,
//    REQUIRES(boost::is_same<typename result_of<typename H::template bind<expected>::type(expected)>::type, void>::value)) const
//  {
//#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
//    typedef expected<void, error_type> result_type;
//    f(*this)(boost::move(*this));
//    return result_type();
//#else
//    return (f(*this)(boost::move(*this)), expected<void, error_type>());
//#endif
//  }
//
//  template <typename H>
//  BOOST_CONSTEXPR expected<typename result_of<typename H::template bind<expected>::type(expected)>::type, error_type>
//  then(BOOST_RV_REF(adaptor_holder<H>) f,
//    REQUIRES(!boost::is_same<typename result_of<typename H::template bind<expected>::type(expected)>::type, void>::value
//        && !boost::is_expected<typename result_of<typename H::template bind<expected>::type(expected)>::type>::value
//        )) const
//  {
//    typedef expected<typename result_of<typename H::template bind<expected>::type(value_type)>::type, error_type> result_type;
//    return result_type(f(*this)(boost::move(*this)));
//  }
//
//  template <typename H>
//  BOOST_CONSTEXPR typename result_of<typename H::template bind<expected>::type(expected)>::type
//  then(BOOST_RV_REF(adaptor_holder<H>) f,
//    REQUIRES(!boost::is_same<typename result_of<typename H::template bind<expected>::type(expected)>::type, void>::value
//        && boost::is_expected<typename result_of<typename H::template bind<expected>::type(expected)>::type>::value
//        )
//    ) const
//  {
//    return f(*this)(boost::move(*this));
//  }

  template <typename F>
  BOOST_CONSTEXPR this_type
  recover(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(error_type)>::type, value_type>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(!valid())
    {
        return this_type(f(contained_err()));
    }
    return *this;
#else
    return ( ! valid()
         ? this_type(f(contained_err()))
         : *this
           );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR this_type recover(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(error_type)>::type, this_type>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(!valid())
    {
        return f(contained_err());
    }
    return *this;
#else
    return ( ! valid()
         ? f(contained_err())
         : *this
         );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR this_type recover(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(error_type)>::type, unexpected_type<error_type> >::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(!valid())
    {
        return f(contained_err());
    }
    return *this;
#else
    return (! valid()
        ? this_type( f(contained_err()) )
        : *this
        );
#endif
    }

  template <typename Ex, typename F>
  this_type catch_exception(BOOST_RV_REF(F) f,
    REQUIRES(
        boost::is_same<typename result_of<F(Ex &)>::type, this_type>::value
        )) const
  {
    try {
      if(!valid()) std::rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return f(e);
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex, typename F>
  this_type catch_exception(BOOST_RV_REF(F) f,
    REQUIRES(
        boost::is_same<typename result_of<F(Ex &)>::type, value_type>::value
        )) const
  {
    try {
      if(!valid()) std::rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return this_type(f(e));
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex>
  bool has_exception() const
  {
    try {
      if(!valid()) std::rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return true;
    }
    catch(...)
    {
    }
    return false;
  }

};

template <typename ErrorType>
class expected<void, ErrorType>
: detail::expected_base<void, ErrorType>
{
public:
  typedef void value_type;
  typedef ErrorType error_type;
  typedef expected_traits<error_type> traits_type;

private:
  typedef expected<void, error_type> this_type;
  typedef detail::expected_base<void, error_type> base_type;

  // Static asserts.
  //typedef boost::is_same<error_type, exceptional_t> is_same_error_exceptional_t;
  typedef boost::is_same<error_type, in_place_t> is_same_error_in_place_t;
  //BOOST_STATIC_ASSERT_MSG( !is_same_error_exceptional_t::value, "bad ErrorType" );
  BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );

  error_type* errorptr() { return std::addressof(base_type::storage.err); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return static_addressof(base_type::storage.err); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR const bool& contained_has_value() const& { return base_type::has_value; }
  bool& contained_has_value() & { return base_type::has_value; }
  bool&& contained_has_value() && { return std::move(base_type::has_value); }

  BOOST_CONSTEXPR const error_type& contained_err() const& { return base_type::storage.err; }
  error_type& contained_err() & { return base_type::storage.err; }
  error_type&& contained_err() && { return std::move(base_type::storage.err); }

#else
  BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  BOOST_CONSTEXPR const error_type& contained_err() const { return base_type::storage.err; }
  error_type& contained_err() { return base_type::storage.err; }
#endif

  // C++03 movable support
  BOOST_COPYABLE_AND_MOVABLE(this_type)

public:

  template <class T>
  struct bind {
    typedef expected<T, error_type> type;
  };

  // Constructors/Destructors/Assignments

  expected(const expected& rhs
    , REQUIRES( std::is_copy_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(detail::only_set_valid, rhs.valid())
  {
    if (! rhs.valid())
    {
      ::new (errorptr()) error_type(rhs.contained_err());
    }
  }

  expected(BOOST_RV_REF(expected) rhs
    , REQUIRES( std::is_move_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_move_constructible<error_type>::value
  )
  : base_type(detail::only_set_valid, rhs.valid())
  {
    if (! rhs.valid())
    {
      ::new (errorptr()) error_type(boost::move(rhs.contained_err()));
    }
  }

  BOOST_CONSTEXPR explicit expected(in_place_t) BOOST_NOEXCEPT
  : base_type()
  {}

  BOOST_CONSTEXPR expected() BOOST_NOEXCEPT
  : base_type()
  {}

  expected(unexpected_type<error_type> const& e
    , REQUIRES(std::is_copy_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(e)
  {}

  template <class Err>
  expected(unexpected_type<Err> const& e
//    , REQUIRES(std::is_copy_constructible<error_type>::value)
  )
//  BOOST_NOEXCEPT_IF(
//    has_nothrow_copy_constructor<error_type>::value
//  )
  : base_type(e)
  {}

  ~expected() = default;

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


  expected& emplace()
  {
    this_type(in_place_t()).swap(*this);
    return *this;
  }

  // Modifiers
  void swap(expected& rhs)
  {
    if (valid())
    {
      if (! rhs.valid())
      {
        error_type t = boost::move(rhs.contained_err());
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
  BOOST_CONSTEXPR explicit operator bool() const BOOST_NOEXCEPT
  {
    return valid();
  }
#endif

  void value() const
  {
    if(!valid())
    {
      traits_type::bad_access(contained_err());
    }
  }

  BOOST_CONSTEXPR error_type const& error() const BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }

  // next factory

  template <typename F>
  BOOST_CONSTEXPR expected<void, error_type> next(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F()>::type, void>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    typedef expected<void, error_type> result_type;
    if(valid())
    {
        f();
        return result_type();
    }
    return get_unexpected();
#else
    typedef expected<typename result_of<F()>::type, error_type> result_type;
    return ( valid()
        ? ( f(), result_type() )
        :  result_type(get_unexpected())
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR expected<typename result_of<F()>::type, error_type>
  next(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F()>::type, void>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    typedef expected<typename result_of<F()>::type, error_type> result_type;
    if(valid())
    {
        return result_type(f());
    }
    return get_unexpected();
#else
    typedef expected<typename result_of<F()>::type, error_type> result_type;
    return ( valid()
        ? result_type(f())
        :  result_type(get_unexpected())
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR expected<void, error_type>
  then(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(expected)>::type, void>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    typedef expected<void, error_type> result_type;
    f(boost::move(*this));
    return result_type();
#else
    return ( f(boost::move(*this)), expected<void, error_type>() );
#endif
  }

  // then factory
  template <typename F>
  BOOST_CONSTEXPR expected<typename result_of<F(expected)>::type, error_type>
  then(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(expected)>::type, void>::value
        && !boost::is_expected<typename result_of<F(expected)>::type>::value
        )) const
  {
    typedef expected<typename result_of<F(expected)>::type, error_type> result_type;
    return result_type(f(boost::move(*this)));
  }

  template <typename F>
  BOOST_CONSTEXPR typename result_of<F(expected)>::type
  then(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(expected)>::type, void>::value
        && boost::is_expected<typename result_of<F(expected)>::type>::value
        )
    ) const
  {
    return f(boost::move(*this));
  }

//  template <typename H>
//  BOOST_CONSTEXPR expected<void, error_type>
//  then(BOOST_RV_REF(adaptor_holder<H>) f,
//    REQUIRES(boost::is_same<typename result_of<typename H::template bind<expected>::type(expected)>::type, void>::value)) const
//  {
//#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
//    typedef expected<void, error_type> result_type;
//    f(*this)(boost::move(*this));
//    return result_type();
//#else
//    return (f(*this)(boost::move(*this)), expected<void, error_type>());
//#endif
//  }
//
//  template <typename H>
//  BOOST_CONSTEXPR expected<typename result_of<typename H::template bind<expected>::type(expected)>::type, error_type>
//  then(BOOST_RV_REF(adaptor_holder<H>) f,
//    REQUIRES(!boost::is_same<typename result_of<typename H::template bind<expected>::type(expected)>::type, void>::value
//        && !boost::is_expected<typename result_of<typename H::template bind<expected>::type(expected)>::type>::value
//        )) const
//  {
//    typedef expected<typename result_of<typename H::template bind<expected>::type(expected)>::type, error_type> result_type;
//    return result_type(f(*this)(boost::move(*this)));
//  }
//
//  template <typename H>
//  BOOST_CONSTEXPR typename result_of<typename H::template bind<expected>::type(expected)>::type
//  then(BOOST_RV_REF(adaptor_holder<H>) f,
//    REQUIRES(!boost::is_same<typename result_of<typename H::template bind<expected>::type(expected)>::type, void>::value
//        && boost::is_expected<typename result_of<typename H::template bind<expected>::type(expected)>::type>::value
//        )
//    ) const
//  {
//    return f(*this)(boost::move(*this));
//  }

  // recover factory

  template <typename F>
  BOOST_CONSTEXPR this_type recover(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(error_type)>::type, value_type>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(! valid())
    {
        return this_type(f(contained_err()));
    }
    return *this;
#else
    return (! valid()
        ? this_type(f(contained_err()))
        : *this
        );
#endif
    }

  template <typename F>
  BOOST_CONSTEXPR this_type recover(BOOST_RV_REF(F) f,
      REQUIRES(! boost::is_same<typename result_of<F(error_type)>::type, value_type>::value)) const
  {
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(!valid())
    {
      return f(contained_err());
    }
    return *this;
#else
    return (! valid()
        ? f(contained_err())
        : *this
        );
#endif
  }

  template <typename Ex, typename F>
  this_type catch_exception(BOOST_RV_REF(F) f,
    REQUIRES(
        boost::is_same<typename result_of<F(Ex &)>::type, this_type>::value
        )) const
  {
    try {
      if(!valid()) std::rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return f(e);
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex, typename F>
  this_type catch_exception(BOOST_RV_REF(F) f,
    REQUIRES(
        boost::is_same<typename result_of<F(Ex &)>::type, value_type>::value
        )) const
  {
    try {
      if(!valid()) std::rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return this_type(f(e));
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex>
  bool has_exception() const
  {
    try {
      if(!valid()) std::rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return true;
    }
    catch(...)
    {
    }
    return false;
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

template <class E>
BOOST_CONSTEXPR bool operator==(const expected<void, E>& x, const expected<void, E>& y)
{
  return (x && y)
    ? true
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

template <class E>
BOOST_CONSTEXPR bool operator<(const expected<void, E>& x, const expected<void, E>& y)
{
  return (x)
    ? (y) ? false : true
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
template<typename T>
BOOST_CONSTEXPR expected<T> make_expected(BOOST_FWD_REF(T) v )
{
  return expected<T>(std::forward<T>(v));
}

BOOST_FORCEINLINE expected<void> make_expected()
{
  return expected<void>();
}



#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
  template<typename T, typename E, typename Arg0, typename Arg1, typename... Args>
  inline expected<T,E> make_expected(BOOST_FWD_REF(Arg0) arg0, BOOST_FWD_REF(Arg1) arg1, BOOST_FWD_REF(Args)... args)
  {
    return expected<T,E>(in_place2, boost::forward<Arg0>(arg0), boost::forward<Arg1>(arg1), boost::forward<Args>(args)...);
  }

  template<typename T, typename Arg0, typename Arg1, typename... Args>
  inline expected<T> make_expected(BOOST_FWD_REF(Arg0) arg0, BOOST_FWD_REF(Arg0) arg1, BOOST_FWD_REF(Args)... args)
  {
    return expected<T>(in_place2, boost::forward<Arg0>(arg0), boost::forward<Arg1>(arg1), boost::forward<Args>(args)...);
  }
#endif

template <typename T>
BOOST_FORCEINLINE expected<T> make_expected_from_current_exception() BOOST_NOEXCEPT
{
  return expected<T>(make_unexpected_from_current_exception());
}

template <typename T>
BOOST_FORCEINLINE expected<T> make_expected_from_exception(std::exception_ptr e) BOOST_NOEXCEPT
{
  return expected<T>(unexpected_type<>(e));
}

template <typename T, typename E>
BOOST_FORCEINLINE expected<T> make_expected_from_exception(E e) BOOST_NOEXCEPT
{
  return expected<T>(unexpected_type<>(e));
}

template <typename T, typename E>
BOOST_FORCEINLINE BOOST_CONSTEXPR
expected<T,decay_t<E>> make_expected_from_error(E e) BOOST_NOEXCEPT
{
  return expected<T, decay_t<E> >(make_unexpected(e));
}

template <typename F>
expected<typename boost::result_of<F()>::type>
BOOST_FORCEINLINE make_expected_from_call(F funct
  , REQUIRES( ! boost::is_same<typename boost::result_of<F()>::type, void>::value)
) BOOST_NOEXCEPT
{
  try
  {
    return make_expected(funct());
  }
  catch (...)
  {
    return make_unexpected_from_current_exception();
  }
}

template <typename F>
inline expected<void>
make_expected_from_call(F funct
  , REQUIRES( boost::is_same<typename boost::result_of<F()>::type, void>::value)
) BOOST_NOEXCEPT
{
  try
  {
    funct();
    return make_expected();
  }
  catch (...)
  {
    return make_unexpected_from_current_exception();
  }
}

} // namespace boost

#if defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
  #undef MAKE_BOOST_FWD_REF_ARG
  #undef MAKE_BOOST_FWD_PARAM
#endif

#undef REQUIRES
#undef T_REQUIRES


#endif // BOOST_EXPECTED_HPP
