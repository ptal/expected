// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013,2014 Vicente J. Botet Escriba
// (C) Copyright 2013 Pierre Talbot

#ifndef BOOST_EXPECTED_EXPECTED_HPP
#define BOOST_EXPECTED_EXPECTED_HPP

#include <boost/expected/config.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/expected/detail/static_addressof.hpp>
#include <boost/expected/detail/constexpr_utility.hpp>

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
#include <type_traits>

# define REQUIRES(...) typename ::boost::enable_if_c<__VA_ARGS__, void*>::type = 0
# define T_REQUIRES(...) typename = typename ::boost::enable_if_c<(__VA_ARGS__)>::type

namespace boost {

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

    // todo - Add implicit/explicit conversion to error_type ?
};

class expected_default_constructed : public std::logic_error
{
  public:
    expected_default_constructed()
    : std::logic_error("Found a default constructed expected.")
    {}
};

// Traits classes
template <typename ErrorType, class Exception>
struct expected_error_traits
{
  typedef ErrorType error_type;
  typedef ErrorType error_storage_type;
  typedef Exception exception_type;

  template <class E>
  static error_storage_type from_error(E const& e)
  {
    return error_storage_type(e);
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
  typedef boost::exception_ptr exception_type;
  typedef boost::exception_ptr error_storage_type;

  template <class E>
  static error_storage_type from_error(E const& e)
  {
    return boost::copy_exception(e);
  }

  static void bad_access(const error_storage_type &e)
  {
    if (e==boost::exception_ptr())
      throw expected_default_constructed();
    boost::rethrow_exception(e);
  }
};

#if ! defined BOOST_NO_CXX11_HDR_EXCEPTION  && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
template <>
struct expected_traits<std::exception_ptr>
{
  typedef std::exception_ptr error_type;
  typedef std::exception_ptr exception_type;
  typedef std::exception_ptr error_storage_type;

  template <class E>
  static error_type from_error(E const& e)
  {
    return std::make_exception_ptr(e);
  }

  static void bad_access(const error_type &e)
  {
    if (e==std::exception_ptr())
      throw expected_default_constructed();
    std::rethrow_exception(e);
  }
};
#endif

struct in_place_t {};
BOOST_CONSTEXPR_OR_CONST in_place_t in_place2 = {};

struct expect_t {};
BOOST_CONSTEXPR_OR_CONST expect_t expect = {};

struct unexpect_t {};
BOOST_CONSTEXPR_OR_CONST unexpect_t unexpect = {};

namespace detail {

template <typename T, typename E, class traits_type=expected_traits<E> >
union trivial_expected_storage
{
  typedef T value_type;
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  error_storage_type err;
  value_type val;

  BOOST_CONSTEXPR trivial_expected_storage()
    BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
  : err()
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_storage_type> const& e)
  : err(e.value())
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_storage_type> && e)
  : err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  template <class... Args>
  BOOST_CONSTEXPR trivial_expected_storage(in_place_t, BOOST_FWD_REF(Args)... args)
  : val(constexpr_forward<Args>(args)...)
  {}

  ~trivial_expected_storage() = default;
};

template <typename E, class traits_type>
union trivial_expected_storage<void, E, traits_type >
{
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  error_storage_type err;
  unsigned char dummy;

  BOOST_CONSTEXPR trivial_expected_storage()
  : err()
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_storage_type> const& e)
  : err(e.value())
  {}
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_storage_type> && e)
  : err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  BOOST_CONSTEXPR trivial_expected_storage(in_place_t)
  : dummy(0)
  {}
  ~trivial_expected_storage() = default;
};

template <typename T, typename E, class traits_type=expected_traits<E> >
union no_trivial_expected_storage
{
  typedef T value_type;
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  error_storage_type err;
  value_type val;

  BOOST_CONSTEXPR no_trivial_expected_storage()
  : err()
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_storage_type> const& e)
  : err(e.value())
  {}
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_storage_type> && e)
  : err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  template <class... Args>
  BOOST_CONSTEXPR no_trivial_expected_storage(in_place_t, BOOST_FWD_REF(Args)... args) //BOOST_NOEXCEPT_IF()
  : val(constexpr_forward<Args>(args)...)
  {}

  ~no_trivial_expected_storage() {};
};

template <typename E, class traits_type>
union no_trivial_expected_storage<void, E, traits_type >
{
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  error_storage_type err;
  unsigned char dummy;

  BOOST_CONSTEXPR no_trivial_expected_storage()
  : err()
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_storage_type> const& e)
  : err(e.value())
  {}
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_storage_type> && e)
  : err(boost::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<Err> const& e)
  : err(traits_type::from_error(e.value()))
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(in_place_t)
  : dummy(0)
  {}

  ~no_trivial_expected_storage() {};
};

BOOST_CONSTEXPR struct only_set_valid_t{} only_set_valid{};

template <typename T, typename E, class traits_type=expected_traits<E> >
struct trivial_expected_base
{
  typedef T value_type;
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  bool has_value;
  trivial_expected_storage<T, E, traits_type> storage;

  BOOST_CONSTEXPR trivial_expected_base()
    //BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
  : has_value(false), storage()
  {}

  BOOST_CONSTEXPR trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value)
  {}

  BOOST_CONSTEXPR trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place2, v)
  {}

  BOOST_CONSTEXPR trivial_expected_base(BOOST_FWD_REF(value_type) v)
  : has_value(true), storage(in_place2, constexpr_move(v))
  {}

  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}

  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<error_type>>(e))
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<Err>>(e))
  {}



  template <class... Args>
  explicit BOOST_CONSTEXPR
  trivial_expected_base(in_place_t, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(in_place2, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit BOOST_CONSTEXPR
  trivial_expected_base(in_place_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(in_place2, il, constexpr_forward<Args>(args)...)
  {}

   ~trivial_expected_base() = default;
};

template <typename E, class traits_type>
struct trivial_expected_base<void, E, traits_type >
{
  typedef void value_type;
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  bool has_value;
  trivial_expected_storage<void, E, traits_type> storage;

  BOOST_CONSTEXPR trivial_expected_base()
  : has_value(false), storage() {}

  BOOST_CONSTEXPR trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value) {}

  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<error_type>>(e))
  {}
  template <class Err>
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  BOOST_CONSTEXPR trivial_expected_base(unexpected_type<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<Err>>(e))
  {}
  BOOST_CONSTEXPR trivial_expected_base(in_place_t)
  : has_value(true), storage(in_place2)
  {}

   ~trivial_expected_base() = default;
};

template <typename T, typename E, class traits_type=expected_traits<E> >
struct no_trivial_expected_base
{
  typedef T value_type;
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  bool has_value;
  no_trivial_expected_storage<T, E, traits_type> storage;

  BOOST_CONSTEXPR no_trivial_expected_base()
    //BOOST_NOEXCEPT_IF(has_nothrow_default_constructor<value_type>::value)
  : has_value(false), storage()
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place2, v)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(BOOST_FWD_REF(value_type) v)
  : has_value(true), storage(in_place2, constexpr_move(v))
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<error_type>>(e))
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<Err>>(e))
  {}

  template <class... Args>
  explicit BOOST_CONSTEXPR
  no_trivial_expected_base(in_place_t, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(in_place2, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit BOOST_CONSTEXPR
  no_trivial_expected_base(in_place_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : has_value(true), storage(in_place2, il, constexpr_forward<Args>(args)...)
  {}

  ~no_trivial_expected_base()
  {
    if (has_value) storage.val.~value_type();
    else storage.err.~error_storage_type();
  }
};

template <typename E, class traits_type>
struct no_trivial_expected_base<void, E, traits_type> {
  typedef void value_type;
  typedef typename traits_type::error_type error_type;
  typedef typename traits_type::error_storage_type error_storage_type;

  bool has_value;
  no_trivial_expected_storage<void, E, traits_type> storage;

  BOOST_CONSTEXPR no_trivial_expected_base()
  : has_value(false), storage() {}

  BOOST_CONSTEXPR no_trivial_expected_base(only_set_valid_t, bool has_value)
  : has_value(has_value) {}


  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<error_type> const& e)
  : has_value(false), storage(e)
  {}
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<error_type>>(e))
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_base(unexpected_type<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected_type<Err>>(e))
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(in_place_t)
  : has_value(true), storage(in_place2)
  {}

  ~no_trivial_expected_base() {
    if (! has_value)
      storage.err.~error_storage_type();
  }
};

template <typename T, typename E, class traits_type=expected_traits<E> >
  using expected_base = typename std::conditional<
    has_trivial_destructor<T>::value && has_trivial_destructor<E>::value,
    trivial_expected_base<T,E,traits_type>,
    no_trivial_expected_base<T,E,traits_type>
  >::type;

} // namespace detail

struct holder;
template <typename ErrorType=std::exception_ptr, typename ValueType=holder>
class expected;

namespace expected_detail
{

  template <class C>
  struct unwrap_result_type;

  template <class E, class T>
  struct unwrap_result_type<expected<E,T>> {
    using type = expected<E, T>;
  };

  template <class E, class T>
  struct unwrap_result_type<expected<E,expected<E,T>>> {
    using type = expected<E, T>;
  };

  template <class C>
  using unwrap_result_type_t = typename unwrap_result_type<C>::type;

}

template <typename T>
struct is_expected : false_type {};
template <typename E, typename T>
struct is_expected<expected<E,T> > : true_type {};

template <typename ErrorType, typename ValueType>
class expected
: private detail::expected_base<ValueType, ErrorType, expected_traits<ErrorType> >
{
public:
  typedef ValueType value_type;
  typedef expected_traits<ErrorType> traits_type;
  typedef typename traits_type::error_type error_type;
  typedef ErrorType error_param_type;
  typedef typename traits_type::error_storage_type error_storage_type;
  using errored_type = boost::unexpected_type<error_type>;

private:
  typedef expected<ErrorType, value_type> this_type;
  typedef detail::expected_base<ValueType, ErrorType, expected_traits<ErrorType> > base_type;

  // Static asserts.
  typedef boost::is_unexpected<value_type> is_unexpected_value_t;
  BOOST_STATIC_ASSERT_MSG( !is_unexpected_value_t::value, "bad ValueType" );
  typedef boost::is_same<value_type, in_place_t> is_same_value_in_place_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_value_in_place_t::value, "bad ValueType" );
  typedef boost::is_same<value_type, unexpect_t> is_same_value_unexpect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_value_unexpect_t::value, "bad ValueType" );
  typedef boost::is_same<value_type, expect_t> is_same_value_expect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_value_expect_t::value, "bad ValueType" );
  typedef boost::is_unexpected<error_param_type> is_unexpected_error_t;
  BOOST_STATIC_ASSERT_MSG( !is_unexpected_error_t::value, "bad ErrorType" );
  typedef boost::is_same<error_param_type, in_place_t> is_same_error_in_place_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );
  typedef boost::is_same<error_param_type, unexpect_t> is_same_error_unexpect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_unexpect_t::value, "bad ErrorType" );
  typedef boost::is_same<error_param_type, expect_t> is_same_error_expect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_expect_t::value, "bad ErrorType" );

  value_type* dataptr() { return std::addressof(base_type::storage.val); }
  BOOST_CONSTEXPR const value_type* dataptr() const { return static_addressof(base_type::storage.val); }
  error_storage_type* errorptr() { return std::addressof(base_type::storage.err); }
  BOOST_CONSTEXPR const error_storage_type* errorptr() const { return static_addressof(base_type::storage.err); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR const bool& contained_has_value() const& { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool& contained_has_value() & { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool&& contained_has_value() && { return std::move(base_type::has_value); }

  BOOST_CONSTEXPR const value_type& contained_val() const& { return base_type::storage.val; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return base_type::storage.val; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return std::move(base_type::storage.val); }

  BOOST_CONSTEXPR const error_storage_type& contained_err() const& { return base_type::storage.err; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_storage_type& contained_err() & { return base_type::storage.err; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_storage_type&& contained_err() && { return std::move(base_type::storage.err); }

#else
  BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  BOOST_CONSTEXPR const value_type& contained_val() const { return base_type::storage.val; }
  value_type& contained_val() { return base_type::storage.val; }
  BOOST_CONSTEXPR const error_storage_type& contained_err() const { return base_type::storage.err; }
  error_storage_type& contained_err() { return base_type::storage.err; }
#endif

  // C++03 movable support
  BOOST_COPYABLE_AND_MOVABLE(this_type)

public:

  template <class T>
  using rebind = expected<error_param_type, T>;

  using type_constructor = expected<error_param_type>;


  // Constructors/Destructors/Assignments

  BOOST_CONSTEXPR expected(
     //REQUIRES(std::is_default_constructible<error_type>::value)
  ) BOOST_NOEXCEPT_IF(
  has_nothrow_default_constructor<error_type>::value
  )
  : base_type()
  {}

  BOOST_CONSTEXPR expected(const value_type& v
    //, REQUIRES(std::is_copy_constructible<value_type>::value)
  )
  BOOST_NOEXCEPT_IF(has_nothrow_copy_constructor<value_type>::value)
  : base_type(v)
  {}

  BOOST_CONSTEXPR expected(BOOST_RV_REF(value_type) v
    //, REQUIRES(std::is_move_constructible<value_type>::value)
  )
  BOOST_NOEXCEPT_IF(
        std::is_nothrow_move_constructible<value_type>::value
  )
  : base_type(constexpr_move(v))
  {}

  expected(const expected& rhs
    //, REQUIRES( std::is_copy_constructible<value_type>::value
      //         && std::is_copy_constructible<error_type>::value)
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
      ::new (errorptr()) error_storage_type(rhs.contained_err());
    }
  }

  expected(BOOST_RV_REF(expected) rhs
    //, REQUIRES( std::is_move_constructible<value_type>::value
      //         && std::is_move_constructible<error_type>::value)
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
      ::new (errorptr()) error_storage_type(boost::move(rhs.contained_err()));
    }
  }

  expected(unexpected_type<error_type> const& e
    //, REQUIRES(std::is_copy_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(e)
  {}
  expected(unexpected_type<error_type> && e
    //, REQUIRES(std::is_move_constructible<error_type>::value)
  )
  //BOOST_NOEXCEPT_IF(
  //  has_nothrow_move_constructor<error_type>::value
  //)
  : base_type(std::forward<unexpected_type<error_type>>(e))
  {}

  template <class Err>
  expected(unexpected_type<Err> const& e
//    , REQUIRES(std::is_copy_constructible<error_type>::value)
  )
  //BOOST_NOEXCEPT_IF(
    //has_nothrow_copy_constructor<error_type>::value
    //std::is_nothrow_copy_constructible<error_type>::value
  //)
  : base_type(e)
  {}
  template <class Err>
  expected(unexpected_type<Err> && e
//    , REQUIRES(std::is_constructible<error_type, Err&&>::value)
  )
  //BOOST_NOEXCEPT_IF(
    //std::is_nothrow_constructible<error_type, Err&&>::value
  //)
  : base_type(std::forward<unexpected_type<Err>>(e))
  {}

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  , T_REQUIRES(std::is_constructible<error_type, Args&...>::value)
#endif
  >
  expected(unexpect_t, BOOST_FWD_REF(Args)... args
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(unexpected_type<error_type>(error_type(std::forward<Args>(args)...)))
  {}


  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    //, T_REQUIRES(std::is_constructible<value_type, decay_t<Args>...>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(in_place_t, BOOST_FWD_REF(Args)... args)
  : base_type(in_place_t{}, boost::constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, std::initializer_list<U> >::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(in_place_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : base_type(in_place_t{}, il, constexpr_forward<Args>(args)...)
  {}

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(expect_t, BOOST_FWD_REF(Args)... args)
  : base_type(in_place_t{}, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, std::initializer_list<U> >::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(expect_t, std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
  : base_type(in_place_t{}, il, constexpr_forward<Args>(args)...)
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

  template <class U, T_REQUIRES(is_same<decay_t<U>, value_type>::value)>
  expected& operator=(BOOST_COPY_ASSIGN_REF(U) value)
  {
    this_type(value).swap(*this);
    return *this;
  }

  template <class U, T_REQUIRES(is_same<decay_t<U>, value_type>::value)>
  expected& operator=(BOOST_RV_REF(U) value)
  {
    this_type(boost::move(value)).swap(*this);
    return *this;
  }

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
    >
  void emplace(BOOST_FWD_REF(Args)... args)
    {
      this_type(in_place_t{}, constexpr_forward<Args>(args)...).swap(*this);
    }

    template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
      //, T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
      >
    void emplace(std::initializer_list<U> il, BOOST_FWD_REF(Args)... args)
    {
      this_type(in_place_t{}, il, constexpr_forward<Args>(args)...).swap(*this);
    }

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
        error_storage_type t = boost::move(rhs.contained_err());
        new (rhs.dataptr()) value_type(boost::move(contained_val()));
        new (errorptr()) error_storage_type(t);
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

#if ! defined BOOST_EXPECTED_NO_CXX11_MOVE_ACCESSORS
  BOOST_CONSTEXPR value_type const& value() const&
  {
    return valid()
      ? contained_val()
      : (
          traits_type::bad_access(contained_err()),
          contained_val()
        )
      ;
  }
  BOOST_CONSTEXPR value_type& value() &
  {
    if (!valid()) traits_type::bad_access(contained_err());
    return contained_val();
  }
  BOOST_CONSTEXPR value_type&& value() &&
  {
    if (!valid()) traits_type::bad_access(contained_err());
    return std::move(contained_val());
  }

#else
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
#endif
#if ! defined BOOST_EXPECTED_NO_CXX11_MOVE_ACCESSORS
  BOOST_CONSTEXPR value_type const& operator*() const& BOOST_NOEXCEPT
  {
    return contained_val();
  }

  BOOST_CONSTEXPR value_type& operator*() & BOOST_NOEXCEPT
  {
    return contained_val();
  }
  BOOST_CONSTEXPR value_type&& operator*() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_val());
  }
#else
  BOOST_CONSTEXPR value_type const& operator*() const BOOST_NOEXCEPT
  {
    return contained_val();
  }

  value_type& operator*() BOOST_NOEXCEPT
  {
    return contained_val();
  }
#endif

  BOOST_CONSTEXPR value_type const* operator->() const BOOST_NOEXCEPT
  {
    return dataptr();
  }

  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type* operator->() BOOST_NOEXCEPT
  {
    return dataptr();
  }

#if ! defined BOOST_EXPECTED_NO_CXX11_MOVE_ACCESSORS
  BOOST_CONSTEXPR error_type const& error() const& BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_CONSTEXPR error_type& error() & BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_CONSTEXPR error_type&& error() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_err());
  }
#else
  BOOST_CONSTEXPR error_type const& error() const BOOST_NOEXCEPT
  {
    return contained_err();
  }
  error_type& error() BOOST_NOEXCEPT
  {
    return contained_err();
  }
#endif


#if ! defined BOOST_EXPECTED_NO_CXX11_MOVE_ACCESSORS
  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const& BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }

  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() && BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(constexpr_move(contained_err()));
  }
#else
  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }
#endif

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
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type value_or(BOOST_FWD_REF(V) v) &&
  {
    return *this
      ? constexpr_move(const_cast<rebind<value_type>&>(*this).contained_val())
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
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type value_or_throw() &&
  {
    return *this
      ? constexpr_move(const_cast<rebind<value_type>&>(*this).contained_val())
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


#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected> unwrap() const&;
  inline BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS expected_detail::unwrap_result_type_t<expected> unwrap() &&;
#else
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected> unwrap() const;
#endif
  template <typename F>
  BOOST_CONSTEXPR rebind<void>
  fmap(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(value_type)>::type, void>::value)) const
  {
    typedef rebind<void> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        f(**this);
        return result_type(in_place_t{});
    }
    return get_unexpected();
#else
    return (valid()
        ? (f(**this), result_type( in_place_t{}))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR rebind<typename result_of<F(value_type)>::type>
  fmap(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(value_type)>::type, void>::value)) const
  {
    typedef rebind<typename result_of<F(value_type)>::type> result_type;
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
  BOOST_CONSTEXPR rebind<void>
  mbind(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(value_type)>::type, void>::value)) const
  {
    typedef rebind<void> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        f(**this);
        return result_type(in_place_t{});
    }
    return get_unexpected();
#else
    return (valid()
        ? (f(**this), result_type( in_place_t{}))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR rebind<typename result_of<F(value_type)>::type>
  mbind(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(value_type)>::type, void>::value
        && !boost::is_expected<typename result_of<F(value_type)>::type>::value
        )) const
  {
    typedef rebind<typename result_of<F(value_type)>::type> result_type;
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
  mbind(BOOST_RV_REF(F) f,
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
  BOOST_CONSTEXPR rebind<void>
  then(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(expected)>::type, void>::value)) const
  {
    typedef rebind<void> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    f(boost::move(*this));
    return result_type(in_place_t{});
#else
    return (f(boost::move(*this)), result_type(in_place_t{}));
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR rebind<typename result_of<F(expected)>::type>
  then(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(expected)>::type, void>::value
        && !boost::is_expected<typename result_of<F(expected)>::type>::value
        )) const
  {
    typedef rebind<typename result_of<F(value_type)>::type> result_type;
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

  template <typename F>
  BOOST_CONSTEXPR this_type
  catch_error(BOOST_RV_REF(F) f,
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
  BOOST_CONSTEXPR this_type catch_error(BOOST_RV_REF(F) f,
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
  BOOST_CONSTEXPR this_type catch_error(BOOST_RV_REF(F) f,
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

template <typename E>
class expected<E,holder> {
public:
  template <class T>
  using type = expected<E,T>;
  template <class T>
  expected<E,T> make(T&& v) {
    return expected<E,T>(std::forward(v));
  }
};

template <typename T>
using exception_or = expected<std::exception_ptr,T>;

template <typename ErrorType>
class expected<ErrorType,void>
: detail::expected_base<void, ErrorType, expected_traits<ErrorType> >
{
public:
  typedef void value_type;
  typedef expected_traits<ErrorType> traits_type;
  typedef typename traits_type::error_type error_type;
  typedef ErrorType error_param_type;
  typedef typename traits_type::error_storage_type error_storage_type;
  using errored_type = boost::unexpected_type<error_type>;

private:
  typedef expected<error_param_type, void> this_type;
  typedef detail::expected_base<void, ErrorType, traits_type> base_type;

  // Static asserts.
  typedef boost::is_unexpected<error_type> is_unexpected_error_t;
  BOOST_STATIC_ASSERT_MSG( !is_unexpected_error_t::value, "bad ErrorType" );
  typedef boost::is_same<error_param_type, in_place_t> is_same_error_in_place_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );
  typedef boost::is_same<error_param_type, unexpect_t> is_same_error_unexpect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_unexpect_t::value, "bad ErrorType" );
  typedef boost::is_same<error_param_type, expect_t> is_same_error_expect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_expect_t::value, "bad ErrorType" );

  error_storage_type* errorptr() { return std::addressof(base_type::storage.err); }
  BOOST_CONSTEXPR const error_storage_type* errorptr() const { return static_addressof(base_type::storage.err); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR const bool& contained_has_value() const& { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool& contained_has_value() & { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool&& contained_has_value() && { return std::move(base_type::has_value); }

  BOOST_CONSTEXPR const error_storage_type& contained_err() const& { return base_type::storage.err; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_storage_type& contained_err() & { return base_type::storage.err; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_storage_type&& contained_err() && { return std::move(base_type::storage.err); }

#else
  BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  BOOST_CONSTEXPR const error_storage_type& contained_err() const { return base_type::storage.err; }
  error_storage_type& contained_err() { return base_type::storage.err; }
#endif

  // C++03 movable support
  BOOST_COPYABLE_AND_MOVABLE(this_type)

public:

  template <class T>
  using rebind = expected<error_param_type, T>;

  using type_constructor = expected<error_param_type>;

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
      ::new (errorptr()) error_storage_type(rhs.contained_err());
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
      ::new (errorptr()) error_storage_type(boost::move(rhs.contained_err()));
    }
  }

  BOOST_CONSTEXPR explicit expected(in_place_t) BOOST_NOEXCEPT
  : base_type(in_place2)
  {}
  BOOST_CONSTEXPR explicit expected(expect_t) BOOST_NOEXCEPT
  : base_type(in_place2)
  {}

  BOOST_CONSTEXPR expected(
     REQUIRES(std::is_default_constructible<error_type>::value)
  ) BOOST_NOEXCEPT_IF(
    has_nothrow_default_constructor<error_type>::value
  )
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
  expected(unexpected_type<error_type> && e
    , REQUIRES(std::is_move_constructible<error_type>::value)
  )
  //BOOST_NOEXCEPT_IF(
  //  has_nothrow_copy_constructor<error_type>::value
  //)
  : base_type(std::forward<unexpected_type<error_type>>(e))
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
  template <class Err>
  expected(unexpected_type<Err> && e
//    , REQUIRES(std::is_copy_constructible<error_type>::value)
  )
//  BOOST_NOEXCEPT_IF(
//    has_nothrow_copy_constructor<error_type>::value
//  )
  : base_type(std::forward<unexpected_type<Err>>(e))
  {}

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  , T_REQUIRES(std::is_constructible<error_type, Args&...>::value)
#endif
  >
  expected(unexpect_t, BOOST_FWD_REF(Args)... args
  )
  BOOST_NOEXCEPT_IF(
    has_nothrow_copy_constructor<error_type>::value
  )
  : base_type(unexpected_type<error_type>(error_type(std::forward<Args>(args)...)))
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


  void emplace()
  {
    this_type(in_place_t{}).swap(*this);
  }

  // Modifiers
  void swap(expected& rhs)
  {
    if (valid())
    {
      if (! rhs.valid())
      {
        error_storage_type t = boost::move(rhs.contained_err());
        new (errorptr()) error_storage_type(t);
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

#if ! defined BOOST_EXPECTED_NO_CXX11_MOVE_ACCESSORS
  BOOST_CONSTEXPR error_storage_type const& error() const& BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_CONSTEXPR error_storage_type& error() & BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_CONSTEXPR error_storage_type&& error() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_err());
  }
#else
  BOOST_CONSTEXPR error_storage_type const& error() const BOOST_NOEXCEPT
  {
    return contained_err();
  }
  error_storage_type & error() BOOST_NOEXCEPT
  {
    return contained_err();
  }
#endif

#if ! defined BOOST_EXPECTED_NO_CXX11_MOVE_ACCESSORS
  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const& BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }

  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() && BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(constexpr_move(contained_err()));
  }
#else
  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }
#endif

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected> unwrap() const&;
  inline BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS expected_detail::unwrap_result_type_t<expected> unwrap() &&;
#else
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected> unwrap() const;
#endif

  // mbind factory

  template <typename F>
  BOOST_CONSTEXPR rebind<void> mbind(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F()>::type, void>::value)) const
  {
    typedef rebind<void> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        f();
        return result_type(in_place_t{});
    }
    return get_unexpected();
#else
    return ( valid()
        ? ( f(), result_type(in_place_t{}) )
        :  result_type(get_unexpected())
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR rebind<typename result_of<F()>::type>
  mbind(BOOST_RV_REF(F) f,
    REQUIRES( ! boost::is_same<typename result_of<F()>::type, void>::value) ) const
  {
    typedef rebind<typename result_of<F()>::type> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    if(valid())
    {
        return result_type(f());
    }
    return get_unexpected();
#else
    return ( valid()
        ? result_type(f())
        :  result_type(get_unexpected())
        );
#endif
  }

  template <typename F>
  BOOST_CONSTEXPR rebind<void>
  then(BOOST_RV_REF(F) f,
    REQUIRES(boost::is_same<typename result_of<F(expected)>::type, void>::value)) const
  {
    typedef rebind<void> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
    f(boost::move(*this));
    return result_type(in_place_t{});
#else
    return ( f(boost::move(*this)), result_type(in_place_t{}) );
#endif
  }

  // then factory
  template <typename F>
  BOOST_CONSTEXPR rebind<typename result_of<F(expected)>::type>
  then(BOOST_RV_REF(F) f,
    REQUIRES(!boost::is_same<typename result_of<F(expected)>::type, void>::value
        && !boost::is_expected<typename result_of<F(expected)>::type>::value
        )) const
  {
    typedef rebind<typename result_of<F(expected)>::type> result_type;
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

  // catch_error factory

  template <typename F>
  BOOST_CONSTEXPR this_type catch_error(BOOST_RV_REF(F) f,
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
  BOOST_CONSTEXPR this_type catch_error(BOOST_RV_REF(F) f,
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
template <class E, class T>
BOOST_CONSTEXPR bool operator==(const expected<E, T>& x, const expected<E, T>& y)
{
  return (x && y)
    ? *x == *y
    : (!x && !y)
      ?  x.get_unexpected() == y.get_unexpected()
      : false;
}

template <class E>
BOOST_CONSTEXPR bool operator==(const expected<E, void>& x, const expected<E, void>& y)
{
  return (x && y)
    ? true
    : (!x && !y)
      ?  x.get_unexpected() == y.get_unexpected()
      : false;
}

template <class E, class T>
BOOST_CONSTEXPR bool operator!=(const expected<E, T>& x, const expected<E, T>& y)
{
  return !(x == y);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator<(const expected<E, T>& x, const expected<E, T>& y)
{
  return (x)
    ? (y) ? *x < *y : false
    : (y) ? true : x.get_unexpected() < y.get_unexpected();
}

template <class E>
BOOST_CONSTEXPR bool operator<(const expected<E, void>& x, const expected<E, void>& y)
{
  return (x)
    ? (y) ? false : false
    : (y) ? true : x.get_unexpected() < y.get_unexpected();
}

template <class E, class T>
BOOST_CONSTEXPR bool operator>(const expected<E, T>& x, const expected<E, T>& y)
{
  return (y < x);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator<=(const expected<E, T>& x, const expected<E, T>& y)
{
  return !(y < x);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator>=(const expected<E, T>& x, const expected<E, T>& y)
{
  return !(x < y);
}

// Relational operators with T
template <class E, class T>
BOOST_CONSTEXPR bool operator==(const expected<E, T>& x, const T& v)
{
  return (x) ? *x == v :  false;
}
template <class E>
BOOST_CONSTEXPR bool operator==(const E& v, const expected<E, void>& x)
{
  return x == v;
}

template <class E, class T>
BOOST_CONSTEXPR bool operator!=(const expected<E, T>& x, const T& v)
{
  return ! (x == v);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator!=(const T& v, const expected<E, T>& x)
{
  return x != v;
}

template <class E, class T>
BOOST_CONSTEXPR bool operator<(const expected<E, T>& x, const T& v)
{
  return (x) ? (*x < v) : true ;
}
template <class E, class T>
BOOST_CONSTEXPR bool operator<(const T& v, const expected<E, T>& x)
{
  return (x) ? (v < x) : false ;
}

template <class E, class T>
BOOST_CONSTEXPR bool operator>(const expected<E, T>& x, const T& v)
{
  return v < x;
}
template <class E, class T>
BOOST_CONSTEXPR bool operator>(const T& v, const expected<E, T>& x)
{
  return x < v;
}

template <class E, class T>
BOOST_CONSTEXPR bool operator<=(const expected<E, T>& x, const T& v)
{
  return ! (v < x);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator<=(const T& v, const expected<E, T>& x)
{
  return ! (x < v);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator>=(const expected<E, T>& x, const T& v)
{
  return ! (x < v);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator>=(const T& v, const expected<E, T>& x)
{
  return ! (v < x);
}

// Relational operators with unexpected_type<E>
template <class E, class T>
BOOST_CONSTEXPR bool operator==(const expected<E, T>& x, const unexpected_type<E>& e)
{
  return (!x) ? x.get_unexpected() == e :  false;
}
template <class E, class T>
BOOST_CONSTEXPR bool operator==(const unexpected_type<E>& e, const expected<E, T>& x)
{
  return (x == e);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator!=(const expected<E, T>& x, const unexpected_type<E>& e)
{
  return ! (x == e);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator!=(const unexpected_type<E>& e , const expected<E, T>& x)
{
  return ! (x == e);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator<(const expected<E, T>& x, const unexpected_type<E>& e)
{
  return (!x) ? (x.get_unexpected() < e) : false ;
}
template <class E, class T>
BOOST_CONSTEXPR bool operator<(const unexpected_type<E>& e, const expected<E, T>& x)
{
  return (!x) ? (e < x.get_unexpected()) : true ;
}

template <class E, class T>
BOOST_CONSTEXPR bool operator>(const expected<E, T>& x, const unexpected_type<E>& e)
{
  return (e <  x);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator>(const unexpected_type<E>& e, const expected<E, T>& x)
{
  return (x <  e);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator<=(const expected<E, T>& x, const unexpected_type<E>& e)
{
  return ! (e < x);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator<=(const unexpected_type<E>& e, const expected<E, T>& x)
{
  return ! (x < e);
}

template <class E, class T>
BOOST_CONSTEXPR bool operator>=(const expected<E, T>& x, const unexpected_type<E>& e)
{
  return ! (e > x);
}
template <class E, class T>
BOOST_CONSTEXPR bool operator>=(const unexpected_type<E>& e, const expected<E, T>& x)
{
  return ! (x > e);
}

// Specialized algorithms
template <class E, class T>
void swap(expected<E,T>& x, expected<E, T>& y) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(x.swap(y)))
{
  x.swap(y);
}

// Factories

template<typename T>
BOOST_CONSTEXPR expected<std::exception_ptr, decay_t<T> > make_expected(BOOST_FWD_REF(T) v )
{
  return expected<std::exception_ptr, decay_t<T> >(constexpr_forward<T>(v));
}

BOOST_FORCEINLINE expected<std::exception_ptr, void> make_expected()
{
  return expected<std::exception_ptr, void>(in_place2);
}

template<typename E>
BOOST_FORCEINLINE expected<E, void> make_expected()
{
  return expected<E, void>(in_place2);
}

template <typename T>
BOOST_FORCEINLINE expected<std::exception_ptr, T> make_expected_from_current_exception() BOOST_NOEXCEPT
{
  return expected<std::exception_ptr, T>(make_unexpected_from_current_exception());
}

template <typename T>
BOOST_FORCEINLINE expected<std::exception_ptr, T> make_expected_from_exception(std::exception_ptr e) BOOST_NOEXCEPT
{
  return expected<std::exception_ptr, T>(unexpected_type<>(e));
}

template <typename T, typename E>
BOOST_FORCEINLINE expected<std::exception_ptr, T> make_expected_from_exception(E e) BOOST_NOEXCEPT
{
  return expected<std::exception_ptr, T>(unexpected_type<>(e));
}

template <typename T, typename E>
BOOST_FORCEINLINE BOOST_CONSTEXPR
expected<decay_t<E>, T> make_expected_from_error(E e) BOOST_NOEXCEPT
{
  return expected<decay_t<E>, T >(make_unexpected(e));
}

template <typename F>
expected<std::exception_ptr, typename boost::result_of<F()>::type>
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
inline expected<std::exception_ptr, void>
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

template <class E, class T>
BOOST_FORCEINLINE BOOST_CONSTEXPR unexpected_type<E> make_unexpected(expected<E,T>& ex)
{
  return unexpected_type<E>(ex.error());
}

namespace expected_detail
{

  // Factories

  template <class E, class T>
  inline BOOST_CONSTEXPR expected<E,T> unwrap(expected<E, expected<E,T> > const& ee)
  {
     return (ee) ? *ee : ee.get_unexpected();
  }
  template <class E, class T>
  inline BOOST_CONSTEXPR expected<E,T> unwrap(expected<E, expected<E,T> >&& ee)
  {
    return (ee) ? std::move(*ee) : ee.get_unexpected();
  }
  template <class E, class T>
  inline BOOST_CONSTEXPR expected<E,T> unwrap(expected<E, T> const& e)
  {
    return e;
  }
  template <class E, class T>
  inline BOOST_CONSTEXPR expected<E,T> unwrap(expected<E, T> && e)
  {
    return std::move(e);
  }

} // namespace expected_detail

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  template <typename E, typename T>
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected<E, T>>
  expected<E, T>::unwrap() const&
  {
    return expected_detail::unwrap(*this);
  }
  template <typename E, typename T>
  inline BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS expected_detail::unwrap_result_type_t<expected<E, T>>
  expected<E, T>::unwrap() &&
  {
    return expected_detail::unwrap(*this);
  }
#else
  template <typename E, typename T>
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected<E, T>>
  expected<E, T>::unwrap() const
  {
    return expected_detail::unwrap(*this);
  }
#endif
} // namespace boost

#undef REQUIRES
#undef T_REQUIRES


#endif // BOOST_EXPECTED_EXPECTED_HPP
