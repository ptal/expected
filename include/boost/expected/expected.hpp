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
#include <boost/expected/detail/requires.hpp>
#include <boost/expected/error_traits.hpp>
#include <boost/expected/bad_expected_access.hpp>
#include <boost/type.hpp>

#ifdef BOOST_EXPECTED_USE_BOOST_HPP
#include <boost/exception_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/utility/swap.hpp>
#endif
#include <boost/static_assert.hpp>

#include <stdexcept>
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <memory>


namespace boost {

//namespace no_adl {
//    template <class T>
//    struct wrapper {
//        wrapper(T&& v) : value(move(v)) {}
//        T unwrap() { return move(value); }
//    private:
//        T value;
//    };
//    template <class T>
//    wrapper<decay_t<T>> wrap(T&& v) { return wrapper<decay_t<T>>(std::forward<T>(v)); }
//}

struct in_place_t {};
BOOST_CONSTEXPR_OR_CONST in_place_t in_place2 = {};

struct expect_t {};
BOOST_CONSTEXPR_OR_CONST expect_t expect = {};

struct unexpect_t {};
BOOST_CONSTEXPR_OR_CONST unexpect_t unexpect = {};

namespace detail {

struct only_set_initialized_t{};
BOOST_CONSTEXPR_OR_CONST only_set_initialized_t only_set_initialized = {};

#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
template<class T, class E>
struct unrestricted_union_emulation_storage
{
  char _bytes[sizeof(T)>sizeof(E) ? sizeof(T) : sizeof(E)];
};
template<class E>
struct unrestricted_union_emulation_storage<void, E>
{
  char _bytes[sizeof(E)];
};
template<class T>
struct unrestricted_union_emulation_storage<T, void>
{
  char _bytes[sizeof(T)];
};
struct boost_expected_unrestricted_union_emulation_default_tag { };
template<class Base, class value_type, class error_type>
struct unrestricted_union_emulation_val_tag
{
  char *_bytes() { return static_cast<unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<Base *>(this))->_bytes; }
  const char *_bytes() const { return static_cast<const unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<const Base *>(this))->_bytes; }
  const value_type &val() const { return *reinterpret_cast<const value_type *>(_bytes()); }
  value_type &val() { return *reinterpret_cast<value_type *>(_bytes()); }
  unrestricted_union_emulation_val_tag() {}
  unrestricted_union_emulation_val_tag(boost_expected_unrestricted_union_emulation_default_tag)
  {
    ::new(&val()) value_type();
  }
  template<class Arg, class... Args> explicit unrestricted_union_emulation_val_tag(Arg&& arg, Args&&... args)
  {
    ::new(&val()) value_type(std::forward<Arg>(arg), std::forward<Args>(args)...);
  }
};
template<class Base, class value_type, class error_type>
struct unrestricted_union_emulation_err_tag
{
  char *_bytes() { return static_cast<unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<Base *>(this))->_bytes; }
  const char *_bytes() const { return static_cast<const unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<const Base *>(this))->_bytes; }
  const error_type &err() const { return *reinterpret_cast<const error_type *>(_bytes()); }
  error_type &err() { return *reinterpret_cast<error_type *>(_bytes()); }
  unrestricted_union_emulation_err_tag() {}
  unrestricted_union_emulation_err_tag(boost_expected_unrestricted_union_emulation_default_tag)
  {
    ::new(&err()) error_type();
  }
  template<class Arg, class... Args> explicit unrestricted_union_emulation_err_tag(Arg&& arg, Args&&... args)
  {
    ::new(&err()) error_type(std::forward<Arg>(arg), std::forward<Args>(args)...);
  }
};
#else
#define boost_expected_unrestricted_union_emulation_default_tag(...) // chances of collision are low
#endif // BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS

template <class T, class E>
#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
struct trivial_expected_storage
  : unrestricted_union_emulation_storage<T, E>,
  unrestricted_union_emulation_val_tag<trivial_expected_storage<T, E>, T, E>,
  unrestricted_union_emulation_err_tag<trivial_expected_storage<T, E>, T, E>
#else
union trivial_expected_storage
#endif
{
  typedef T value_type;
  typedef E error_type;

#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<trivial_expected_storage<T, E>, T, E> _err;
  typedef unrestricted_union_emulation_val_tag<trivial_expected_storage<T, E>, T, E> _val;
#else
  value_type _val;
  error_type _err;
  BOOST_CONSTEXPR const error_type &err() const { return _err; }
  error_type &err() { return _err; }
  BOOST_CONSTEXPR const value_type &val() const { return _val; }
  value_type &val() { return _val; }
#endif

  BOOST_EXPECTED_0_REQUIRES(
        std::is_default_constructible<value_type>::value
  )
  BOOST_CONSTEXPR trivial_expected_storage()
    BOOST_NOEXCEPT_IF(std::is_nothrow_default_constructible<value_type>::value)
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_type> const& e)
  : _err(e.value())
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_type> && e)
  : _err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<Err> const& e)
  : _err(error_traits<error_type>::make_error(e.value()))
  {}

  BOOST_CONSTEXPR trivial_expected_storage(in_place_t)
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  template <class... Args>
  BOOST_CONSTEXPR trivial_expected_storage(in_place_t, Args&&... args)
  : _val(constexpr_forward<Args>(args)...)
  {}

  ~trivial_expected_storage() = default;
};

template <typename E>
#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
struct trivial_expected_storage<void, E>
  : unrestricted_union_emulation_storage<void, E>,
  unrestricted_union_emulation_err_tag<trivial_expected_storage<void, E>, void, E>
#else
union trivial_expected_storage<void, E>
#endif
{
  typedef E error_type;

  unsigned char dummy;
#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<trivial_expected_storage<void, E>, void, E> _err;
#else
  error_type _err;
  BOOST_CONSTEXPR const error_type &err() const { return _err; }
  error_type &err() { return _err; }
#endif

  BOOST_CONSTEXPR trivial_expected_storage()
  : dummy(0)
  //: _err(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_type> const& e)
  : _err(e.value())
  {}
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<error_type> && e)
  : _err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR trivial_expected_storage(unexpected_type<Err> const& e)
  : _err(error_traits<error_type>::make_error(e.value()))
  {}

  BOOST_CONSTEXPR trivial_expected_storage(in_place_t)
  : dummy(0)
  {}
  ~trivial_expected_storage() = default;
};

template <typename T, typename E >
#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
struct no_trivial_expected_storage
  : unrestricted_union_emulation_storage<T, E>,
  unrestricted_union_emulation_val_tag<no_trivial_expected_storage<T, E>, T, E>,
  unrestricted_union_emulation_err_tag<no_trivial_expected_storage<T, E>, T, E>
#else
union no_trivial_expected_storage
#endif
{
  typedef T value_type;
  typedef E error_type;

#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<no_trivial_expected_storage<T, E>, T, E> _err;
  typedef unrestricted_union_emulation_val_tag<no_trivial_expected_storage<T, E>, T, E> _val;
#else
  unsigned char dummy;
  value_type _val;
  error_type _err;
  BOOST_CONSTEXPR const error_type &err() const { return _err; }
  error_type &err() { return _err; }
  BOOST_CONSTEXPR const value_type &val() const { return _val; }
  value_type &val() { return _val; }
#endif

  BOOST_CONSTEXPR no_trivial_expected_storage(only_set_initialized_t)
  : dummy(0)
  {}
  BOOST_EXPECTED_0_REQUIRES(
        std::is_default_constructible<value_type>::value
  )
  BOOST_CONSTEXPR no_trivial_expected_storage()
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(value_type const& v)
  : _val(v)
  {}
  BOOST_CONSTEXPR no_trivial_expected_storage(value_type && v)
  : _val(std::move(v))
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_type> const& e)
  : _err(e.value())
  {}
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_type> && e)
  : _err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<Err> const& e)
  : _err(error_traits<error_type>::make_error(e.value()))
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(in_place_t) //BOOST_NOEXCEPT_IF()
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  template <class... Args>
  BOOST_CONSTEXPR no_trivial_expected_storage(in_place_t, Args&&... args) //BOOST_NOEXCEPT_IF()
  : _val(constexpr_forward<Args>(args)...)
  {}

  ~no_trivial_expected_storage() {};
};

template <typename E>
#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
struct no_trivial_expected_storage<void, E>
  : unrestricted_union_emulation_storage<void, E>,
  unrestricted_union_emulation_err_tag<no_trivial_expected_storage<void, E>, void, E>
#else
union no_trivial_expected_storage<void, E>
#endif
{
  typedef E error_type;

  unsigned char dummy;
#ifdef BOOST_EXPECTED_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<no_trivial_expected_storage<void, E>, void, E> _err;
#else
  error_type _err;
  BOOST_CONSTEXPR const error_type &err() const { return _err; }
  error_type &err() { return _err; }
#endif

  BOOST_CONSTEXPR no_trivial_expected_storage(only_set_initialized_t)
  : dummy(0)
  {}
  BOOST_CONSTEXPR no_trivial_expected_storage()
  : dummy(0)
  //: _err(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_type> const& e)
  : _err(e.value())
  {}
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<error_type> && e)
  : _err(std::move(e.value()))
  {}

  template <class Err>
  BOOST_CONSTEXPR no_trivial_expected_storage(unexpected_type<Err> const& e)
  : _err(error_traits<error_type>::make_error(e.value()))
  {}

  BOOST_CONSTEXPR no_trivial_expected_storage(in_place_t)
  : dummy(0)
  {}

  ~no_trivial_expected_storage() {};
};


template <typename T, typename E >
struct trivial_expected_base
{
  typedef T value_type;
  typedef E error_type;

  bool has_value;
  trivial_expected_storage<T, E> storage;

  BOOST_EXPECTED_0_REQUIRES(
        std::is_default_constructible<value_type>::value
  )
  BOOST_CONSTEXPR trivial_expected_base()
    BOOST_NOEXCEPT_IF(std::is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  BOOST_CONSTEXPR trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place2, v)
  {}

  BOOST_CONSTEXPR trivial_expected_base(value_type&& v)
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
  trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place2, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit BOOST_CONSTEXPR
  trivial_expected_base(in_place_t, std::initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place2, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr() { return std::addressof(storage.val()); }
  BOOST_CONSTEXPR const value_type* dataptr() const { return detail::static_addressof(storage.val()); }
  error_type* errorptr() { return std::addressof(storage.err()); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return detail::static_addressof(storage.err()); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR const value_type& contained_val() const& { return storage.val(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return storage.val(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return std::move(storage.val()); }

  BOOST_CONSTEXPR const error_type& contained_err() const& { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return std::move(storage.err()); }

#else
  BOOST_CONSTEXPR const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  BOOST_CONSTEXPR const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

//  BOOST_EXPECTED_0_REQUIRES(
//        std::is_copy_constructible<value_type>::value &&
//        std::is_copy_constructible<error_type>::value
//  )
  trivial_expected_base(const trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_copy_constructible<value_type>::value &&
          std::is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(rhs.contained_val());
      }
      else
      {
        ::new (errorptr()) error_type(rhs.contained_err());
      }
      has_value = rhs.has_value;
    }

//  BOOST_EXPECTED_0_REQUIRES(
//      std::is_move_constructible<value_type>::value &&
//      std::is_move_constructible<error_type>::value
//  )
  trivial_expected_base(trivial_expected_base&& rhs
    )
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_move_constructible<value_type>::value &&
          std::is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(std::move(rhs.contained_val()));
      }
      else
      {
        ::new (errorptr()) error_type(std::move(rhs.contained_err()));
      }
      has_value = rhs.has_value;
    }

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
  : has_value(true) {}

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

  // Access
  error_type* errorptr() { return std::addressof(storage.err()); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return detail::static_addressof(storage.err()); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  BOOST_CONSTEXPR const error_type& contained_err() const& { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return std::move(storage.err()); }

#else
  BOOST_CONSTEXPR const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

//  BOOST_EXPECTED_0_REQUIRES(
//        std::is_copy_constructible<error_type>::value
//  )
  trivial_expected_base(const trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (errorptr()) error_type(rhs.contained_err());
      }
      has_value = rhs.has_value;
    }

//  BOOST_EXPECTED_0_REQUIRES(
//      std::is_move_constructible<error_type>::value
//  )
  trivial_expected_base(trivial_expected_base&& rhs
    )
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (errorptr()) error_type(std::move(rhs.contained_err()));
      }
      has_value = rhs.has_value;
    }

   ~trivial_expected_base() = default;
};

template <typename T, typename E >
struct no_trivial_expected_base
{
  typedef T value_type;
  typedef E error_type;

  bool has_value;
  no_trivial_expected_storage<T, E> storage;

  BOOST_EXPECTED_0_REQUIRES(
        std::is_default_constructible<value_type>::value
  )
  BOOST_CONSTEXPR no_trivial_expected_base()
    //BOOST_NOEXCEPT_IF(std::is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place2, v)
  {}

  BOOST_CONSTEXPR no_trivial_expected_base(value_type&& v)
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
  no_trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place2, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit BOOST_CONSTEXPR
  no_trivial_expected_base(in_place_t, std::initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place2, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr() { return std::addressof(storage.val()); }
  BOOST_CONSTEXPR const value_type* dataptr() const { return detail::static_addressof(storage.val()); }
  error_type* errorptr() { return std::addressof(storage.err()); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return detail::static_addressof(storage.err()); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  BOOST_CONSTEXPR const value_type& contained_val() const& { return storage.val(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return storage.val(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return std::move(storage.val()); }

  BOOST_CONSTEXPR const error_type& contained_err() const& { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return std::move(storage.err()); }

#else
  BOOST_CONSTEXPR const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  BOOST_CONSTEXPR const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

//  BOOST_EXPECTED_0_REQUIRES(
//        std::is_copy_constructible<value_type>::value &&
//        std::is_copy_constructible<error_type>::value
//  )
  no_trivial_expected_base(const no_trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_copy_constructible<value_type>::value &&
          std::is_nothrow_copy_constructible<error_type>::value
        )
        : has_value(rhs.has_value), storage(only_set_initialized)
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(rhs.contained_val());
      }
      else
      {
        ::new (errorptr()) error_type(rhs.contained_err());
      }
      //has_value = rhs.has_value;
    }

//  BOOST_EXPECTED_0_REQUIRES(
//      std::is_move_constructible<value_type>::value &&
//      std::is_move_constructible<error_type>::value
//  )
  no_trivial_expected_base(no_trivial_expected_base&& rhs
    )
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_move_constructible<value_type>::value &&
          std::is_nothrow_move_constructible<error_type>::value
        )
        : has_value(rhs.has_value), storage(only_set_initialized)
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(std::move(rhs.contained_val()));
      }
      else
      {
        ::new (errorptr()) error_type(std::move(rhs.contained_err()));
      }
      //has_value = rhs.has_value;
    }

  ~no_trivial_expected_base()
  {
    if (has_value) storage.val().~value_type();
    else storage.err().~error_type();
  }
};

template <typename E>
struct no_trivial_expected_base<void, E> {
  typedef void value_type;
  typedef E error_type;

  bool has_value;
  no_trivial_expected_storage<void, E> storage;

  BOOST_CONSTEXPR no_trivial_expected_base()
  : has_value(true) {}

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

  // Access
  error_type* errorptr() { return std::addressof(storage.err()); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return detail::static_addressof(storage.err()); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  BOOST_CONSTEXPR const error_type& contained_err() const& { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return std::move(storage.err()); }

#else
  BOOST_CONSTEXPR const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

//  BOOST_EXPECTED_0_REQUIRES(
//        std::is_copy_constructible<error_type>::value
//  )
  no_trivial_expected_base(const no_trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (errorptr()) error_type(rhs.contained_err());
      }
      has_value = rhs.has_value;
    }

//  BOOST_EXPECTED_0_REQUIRES(
//      std::is_move_constructible<error_type>::value
//  )
  no_trivial_expected_base(no_trivial_expected_base&& rhs
    )
        BOOST_NOEXCEPT_IF(
          std::is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (errorptr()) error_type(std::move(rhs.contained_err()));
      }
      has_value = rhs.has_value;
    }

  ~no_trivial_expected_base() {
    if (! has_value)
      storage.err().~error_type();
  }
};

template <typename T, typename E >
  using expected_base = typename std::conditional<
    std::is_trivially_destructible<T>::value && std::is_trivially_destructible<E>::value,
    trivial_expected_base<T,E>,
    no_trivial_expected_base<T,E>
  >::type;

} // namespace detail

struct holder;
template <typename ValueType=holder, typename ErrorType=std::exception_ptr>
class expected;

namespace expected_detail
{

  template <class C>
  struct unwrap_result_type;

  template <class T, class E>
  struct unwrap_result_type<expected<T,E>> {
    using type = expected<T,E>;
  };

  template <class T, class E>
  struct unwrap_result_type<expected<expected<T,E>,E>> {
    using type = expected<T,E>;
  };

  template <class C>
  using unwrap_result_type_t = typename unwrap_result_type<C>::type;

}

template <typename T>
struct is_expected : std::false_type {};
template <class T, class E>
struct is_expected<expected<T,E>> : std::true_type {};

template <typename ValueType, typename ErrorType>
class expected
: private detail::expected_base<ValueType, ErrorType >
{
public:
  typedef ValueType value_type;
  typedef ErrorType error_type;
  using errored_type = boost::unexpected_type<error_type>;

private:
  typedef expected<value_type, ErrorType> this_type;
  typedef detail::expected_base<ValueType, ErrorType> base_type;

  // Static asserts.
  typedef boost::is_unexpected<value_type> is_unexpected_value_t;
  BOOST_STATIC_ASSERT_MSG( !is_unexpected_value_t::value, "bad ValueType" );
  typedef std::is_same<value_type, in_place_t> is_same_value_in_place_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_value_in_place_t::value, "bad ValueType" );
  typedef std::is_same<value_type, unexpect_t> is_same_value_unexpect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_value_unexpect_t::value, "bad ValueType" );
  typedef std::is_same<value_type, expect_t> is_same_value_expect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_value_expect_t::value, "bad ValueType" );
  typedef boost::is_unexpected<error_type> is_unexpected_error_t;
  BOOST_STATIC_ASSERT_MSG( !is_unexpected_error_t::value, "bad ErrorType" );
  typedef std::is_same<error_type, in_place_t> is_same_error_in_place_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );
  typedef std::is_same<error_type, unexpect_t> is_same_error_unexpect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_unexpect_t::value, "bad ErrorType" );
  typedef std::is_same<error_type, expect_t> is_same_error_expect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_expect_t::value, "bad ErrorType" );

  value_type* dataptr() { return std::addressof(base_type::storage.val()); }
  BOOST_CONSTEXPR const value_type* dataptr() const { return detail::static_addressof(base_type::storage.val()); }
  error_type* errorptr() { return std::addressof(base_type::storage.err()); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return detail::static_addressof(base_type::storage.err()); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR const bool& contained_has_value() const& { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool& contained_has_value() & { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool&& contained_has_value() && { return std::move(base_type::has_value); }

  BOOST_CONSTEXPR const value_type& contained_val() const& { return base_type::storage.val(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return base_type::storage.val(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return std::move(base_type::storage.val()); }

  BOOST_CONSTEXPR const error_type& contained_err() const& { return base_type::storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return base_type::storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return std::move(base_type::storage.err()); }

#else
  BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  BOOST_CONSTEXPR const value_type& contained_val() const { return base_type::storage.val(); }
  value_type& contained_val() { return base_type::storage.val(); }
  BOOST_CONSTEXPR const error_type& contained_err() const { return base_type::storage.err(); }
  error_type& contained_err() { return base_type::storage.err(); }
#endif

#ifdef BOOST_EXPECTED_USE_BOOST_HPP
  // C++03 movable support
  BOOST_COPYABLE_AND_MOVABLE(this_type)
#endif

public:

  // Using a template alias here causes an ICE in VS2013 and VS14 CTP 3
  // so back to the old fashioned way
  template <class T>
  struct rebind
  {
    typedef expected<T, error_type> type;
  };

  using type_constructor = expected<holder, error_type>;


  // Constructors/Destructors/Assignments

  BOOST_EXPECTED_0_REQUIRES(
      std::is_default_constructible<value_type>::value
  )
  BOOST_CONSTEXPR expected()
      BOOST_NOEXCEPT_IF(
          std::is_nothrow_default_constructible<value_type>::value
      )
  : base_type()
  {}

  BOOST_EXPECTED_0_REQUIRES(
      std::is_copy_constructible<value_type>::value
  )
  BOOST_CONSTEXPR expected(const value_type& v)
      BOOST_NOEXCEPT_IF(
          std::is_nothrow_copy_constructible<value_type>::value
      )
  : base_type(v)
  {}

  BOOST_EXPECTED_0_REQUIRES(
    std::is_move_constructible<value_type>::value
  )
  BOOST_CONSTEXPR expected(value_type&& v  )
      BOOST_NOEXCEPT_IF(
            std::is_nothrow_move_constructible<value_type>::value
      )
  : base_type(constexpr_move(v))
  {}

//  BOOST_EXPECTED_0_REQUIRES(
//      std::is_copy_constructible<value_type>::value &&
//      std::is_copy_constructible<error_type>::value
//  )
  expected(const expected& rhs)
      BOOST_NOEXCEPT_IF(
        std::is_nothrow_copy_constructible<value_type>::value &&
        std::is_nothrow_copy_constructible<error_type>::value
      )
  : base_type(rhs)
  {
  }

//  BOOST_EXPECTED_0_REQUIRES(
//      std::is_move_constructible<value_type>::value &&
//      std::is_move_constructible<error_type>::value
//  )
  expected(expected&& rhs
  )
      BOOST_NOEXCEPT_IF(
        std::is_nothrow_move_constructible<value_type>::value &&
        std::is_nothrow_move_constructible<error_type>::value
      )
  : base_type(std::forward<expected>(rhs))
  {
  }

  BOOST_EXPECTED_0_REQUIRES(
      std::is_copy_constructible<error_type>::value
  )
  expected(unexpected_type<error_type> const& e)
      BOOST_NOEXCEPT_IF(
        std::is_nothrow_copy_constructible<error_type>::value
      )
  : base_type(e)
  {}
  BOOST_EXPECTED_0_REQUIRES(std::is_move_constructible<error_type>::value)
  expected(unexpected_type<error_type> && e)
      BOOST_NOEXCEPT_IF(
        std::is_nothrow_move_constructible<error_type>::value
      )
  : base_type(std::forward<unexpected_type<error_type>>(e))
  {}

  template <class Err
    , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<error_type, Err>::value)
  >
  expected(unexpected_type<Err> const& e)
      BOOST_NOEXCEPT_IF((
        std::is_nothrow_constructible<error_type, Err>::value
      ))
  : base_type(e)
  {}
  template <class Err
    //, BOOST_EXPECTED_T_REQUIRES(std::is_constructible<error_type, Err&&>::value)
  >
  expected(unexpected_type<Err> && e
  )
  //BOOST_NOEXCEPT_IF(
    //std::is_nothrow_constructible<error_type, Err&&>::value
  //)
  : base_type(std::forward<unexpected_type<Err>>(e))
  {}

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<error_type, Args&...>::value)
#endif
  >
  expected(unexpect_t, Args&&... args
  )
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(unexpected_type<error_type>(error_type(std::forward<Args>(args)...)))
  {}


  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<value_type, decay_t<Args>...>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(in_place_t, Args&&... args)
  : base_type(in_place_t{}, boost::constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<value_type, std::initializer_list<U>>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(in_place_t, std::initializer_list<U> il, Args&&... args)
  : base_type(in_place_t{}, il, constexpr_forward<Args>(args)...)
  {}

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(expect_t, Args&&... args)
  : base_type(in_place_t{}, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<value_type, std::initializer_list<U>>::value)
#endif
    >
  BOOST_CONSTEXPR explicit expected(expect_t, std::initializer_list<U> il, Args&&... args)
  : base_type(in_place_t{}, il, constexpr_forward<Args>(args)...)
  {}

  ~expected() = default;

  // Assignments
  expected& operator=(expected const& e)
  {
    this_type(e).swap(*this);
    return *this;
  }

  expected& operator=(expected&& e)
  {
    this_type(std::move(e)).swap(*this);
    return *this;
  }

  template <class U, BOOST_EXPECTED_T_REQUIRES(std::is_same<decay_t<U>, value_type>::value)>
  expected& operator=(U const& value)
  {
    this_type(value).swap(*this);
    return *this;
  }

  template <class U, BOOST_EXPECTED_T_REQUIRES(std::is_same<decay_t<U>, value_type>::value)>
  expected& operator=(U&& value)
  {
    this_type(std::move(value)).swap(*this);
    return *this;
  }

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<value_type, Args&...>::value)
#endif
    >
  void emplace(Args&&... args)
    {
      this_type(in_place_t{}, constexpr_forward<Args>(args)...).swap(*this);
    }

    template <class U, class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
      , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<value_type, std::initializer_list<U>, Args&...>::value)
#endif
      >
    void emplace(std::initializer_list<U> il, Args&&... args)
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
        using std::swap;
        swap(contained_val(), rhs.contained_val());
      }
      else
      {
        error_type t = std::move(rhs.contained_err());
        ::new (rhs.dataptr()) value_type(std::move(contained_val()));
        ::new (errorptr()) error_type(t);
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
        using std::swap;
        swap(contained_err(), rhs.contained_err());
      }
    }
  }

  // Observers
  BOOST_CONSTEXPR bool valid() const BOOST_NOEXCEPT
  {
    return contained_has_value();
  }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
  BOOST_CONSTEXPR bool operator !() const BOOST_NOEXCEPT
  {
    return !valid();
  }
  BOOST_CONSTEXPR explicit operator bool() const BOOST_NOEXCEPT
  {
    return valid();
  }
#endif

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  BOOST_CONSTEXPR value_type const& value() const&
  {
    return valid()
      ? contained_val()
      : (
          error_traits<error_type>::rethrow(contained_err()),
          contained_val()
        )
      ;
  }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type& value() &
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return contained_val();
  }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type&& value() &&
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return std::move(contained_val());
  }

#else
  value_type& value()
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return contained_val();
  }

  BOOST_CONSTEXPR value_type const& value() const
  {
    return valid()
      ? contained_val()
      : (
          error_traits<error_type>::rethrow(contained_err()),
          contained_val()
        )
      ;
  }
#endif
#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR value_type const& operator*() const& BOOST_NOEXCEPT
  {
    return contained_val();
  }

  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type& operator*() & BOOST_NOEXCEPT
  {
    return contained_val();
  }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type&& operator*() && BOOST_NOEXCEPT
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

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR error_type const& error() const& BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS error_type& error() & BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS error_type&& error() && BOOST_NOEXCEPT
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


#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected() const& BOOST_NOEXCEPT
  {
    return unexpected_type<error_type>(contained_err());
  }

  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS unexpected_type<error_type> get_unexpected() && BOOST_NOEXCEPT
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
  BOOST_CONSTEXPR value_type value_or(V&& v) const&
  {
    return *this
      ? **this
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class V>
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS value_type value_or(V&& v) &&
  {
    return *this
      ? constexpr_move(const_cast<typename rebind<value_type>::type&>(*this).contained_val())
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
      ? constexpr_move(const_cast<typename rebind<value_type>::type&>(*this).contained_val())
      : throw Exception(contained_err());
  }

# else

  template <class V>
  BOOST_CONSTEXPR value_type value_or(V&& v) const {
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
  typename rebind<void>::type
  catch_all_type_void(F&& f)
  {
    typedef typename rebind<void>::type result_type;
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    try {
#endif
      f(std::move(**this));
      return result_type(in_place_t{});
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }

  template <typename F>
  typename std::result_of<F(value_type)>::type
  catch_all_type_type(F&& f)
  {
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    try {
#endif
      return f(std::move(**this));
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<typename std::result_of<F(value_type)>::type>::type
  catch_all_type_etype(F&& f)
  {
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    //typedef typename rebind<typename std::result_of<F(value_type)>::type>::type result_type;
    try {
#endif
      return f(std::move(**this));
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<void>::type
  catch_all_etype_void(F&& f)
  {
    typedef typename rebind<void>::type result_type;
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    try {
#endif
      f(std::move(*this));
      return result_type(in_place_t{});
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }

  template <typename F>
  typename std::result_of<F(expected)>::type
  catch_all_etype_type(F&& f)
  {
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    //typedef typename std::result_of<F(expected)>::type result_type;
    try {
#endif
      return f(std::move(*this));
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<typename std::result_of<F(expected)>::type>::type
  catch_all_etype_etype(F&& f)
  {
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    //typedef typename rebind<typename std::result_of<F(expected)>::type>::type result_type;
    try {
#endif
      return f(std::move(*this));
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }


  template <typename F>
  typename rebind<void>::type
  map(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(value_type)>::type, void>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return catch_all_type_void(std::forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<void>::type result_type;
    return (valid()
        ? catch_all_type_void(std::forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  typename rebind<typename std::result_of<F(value_type)>::type>::type
  map(F&& f,
    BOOST_EXPECTED_REQUIRES(!std::is_same<typename std::result_of<F(value_type)>::type, void>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
      return catch_all_type_etype(std::forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<typename std::result_of<F(value_type)>::type>::type result_type;
    return (valid()
        ? catch_all_type_etype(std::forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

//  template <typename F>
//  typename rebind<void>::type
//  bind(F&& f,
//    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(value_type)>::type, void>::value))
//  {
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        return catch_all_type_void(std::forward<F>(f));
//    }
//    return get_unexpected();
//#else
//    typedef typename rebind<void>::type result_type;
//    return (valid()
//        ? catch_all_type_void(std::forward<F>(f))
//        : result_type( get_unexpected() )
//        );
//#endif
//  }
//
//  template <typename F>
//  typename rebind<typename std::result_of<F(value_type)>::type>::type
//  bind(F&& f,
//    BOOST_EXPECTED_REQUIRES(!std::is_same<typename std::result_of<F(value_type)>::type, void>::value
//        && !boost::is_expected<typename std::result_of<F(value_type)>::type>::value
//        ))
//  {
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        return catch_all_type_etype(std::forward<F>(f));
//    }
//    return get_unexpected();
//#else
//    typedef typename rebind<typename std::result_of<F(value_type)>::type>::type result_type;
//    return (valid()
//        ? catch_all_type_etype(std::forward<F>(f))
//        : result_type( get_unexpected() )
//        );
//#endif
//  }

  template <typename F>
  typename std::result_of<F(value_type)>::type
  bind(F&& f,
    BOOST_EXPECTED_REQUIRES(boost::is_expected<typename std::result_of<F(value_type)>::type>::value
        )
    )
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
      return catch_all_type_type(std::forward<F>(f));

    }
    return get_unexpected();
#else
     return valid()
         ? catch_all_type_type(std::forward<F>(f))
         : typename std::result_of<F(value_type)>::type(get_unexpected());
#endif
  }

  template <typename F>
  typename rebind<void>::type
  then(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(expected)>::type, void>::value))
  {
    //typedef typename rebind<void>::type result_type;
    return catch_all_etype_void(std::forward<F>(f));
  }

  template <typename F>
  typename rebind<typename std::result_of<F(expected)>::type>::type
  then(F&& f,
    BOOST_EXPECTED_REQUIRES(!std::is_same<typename std::result_of<F(expected)>::type, void>::value
        && !boost::is_expected<typename std::result_of<F(expected)>::type>::value
        ))
  {
    return catch_all_etype_etype(std::forward<F>(f));
    //typedef typename rebind<typename std::result_of<F(value_type)>::type>::type result_type;
    //return result_type(f(std::move(*this)));
  }

  template <typename F>
  typename std::result_of<F(expected)>::type
  then(F&& f,
    BOOST_EXPECTED_REQUIRES(boost::is_expected<typename std::result_of<F(expected)>::type>::value)
    )
  {
    return catch_all_etype_type(std::forward<F>(f));
    //return f(std::move(*this));
  }

  template <typename F>
  this_type
  catch_error(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(error_type)>::type, value_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
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
  this_type catch_error(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(error_type)>::type, this_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
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
  this_type catch_error(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(error_type)>::type, unexpected_type<error_type>>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
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
  this_type catch_exception(F&& f,
    BOOST_EXPECTED_REQUIRES(
        std::is_same<typename std::result_of<F(Ex &)>::type, this_type>::value
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
  this_type catch_exception(F&& f,
    BOOST_EXPECTED_REQUIRES(
        std::is_same<typename std::result_of<F(Ex &)>::type, value_type>::value
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
class expected<holder,E> {
public:
  template <class T>
  using type = expected<T, E>;
  template <class T>
  expected<T,E> make(T&& v) {
    return expected<T,E>(std::forward(v));
  }
};

template <typename T>
using exception_or = expected<T, std::exception_ptr>;

template <typename ErrorType>
class expected<void,ErrorType>
: detail::expected_base<void, ErrorType >
{
public:
  typedef void value_type;
  typedef ErrorType error_type;
  using errored_type = boost::unexpected_type<error_type>;

private:
  typedef expected<void, error_type> this_type;
  typedef detail::expected_base<void, ErrorType> base_type;

  // Static asserts.
  typedef boost::is_unexpected<error_type> is_unexpected_error_t;
  BOOST_STATIC_ASSERT_MSG( !is_unexpected_error_t::value, "bad ErrorType" );
  typedef std::is_same<error_type, in_place_t> is_same_error_in_place_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_in_place_t::value, "bad ErrorType" );
  typedef std::is_same<error_type, unexpect_t> is_same_error_unexpect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_unexpect_t::value, "bad ErrorType" );
  typedef std::is_same<error_type, expect_t> is_same_error_expect_t;
  BOOST_STATIC_ASSERT_MSG( !is_same_error_expect_t::value, "bad ErrorType" );

  error_type* errorptr() { return std::addressof(base_type::storage.err()); }
  BOOST_CONSTEXPR const error_type* errorptr() const { return detail::static_addressof(base_type::storage.err()); }

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CONSTEXPR const bool& contained_has_value() const& { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool& contained_has_value() & { return base_type::has_value; }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  bool&& contained_has_value() && { return std::move(base_type::has_value); }

  BOOST_CONSTEXPR const error_type& contained_err() const& { return base_type::storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return base_type::storage.err(); }
  BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return std::move(base_type::storage.err()); }

#else
  BOOST_CONSTEXPR const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  BOOST_CONSTEXPR const error_type& contained_err() const { return base_type::storage.err(); }
  error_type& contained_err() { return base_type::storage.err(); }
#endif

#ifdef BOOST_EXPECTED_USE_BOOST_HPP
  // C++03 movable support
  BOOST_COPYABLE_AND_MOVABLE(this_type)
#endif

public:

  // Using a template alias here causes an ICE in VS2013 and VS14 CTP 3
  // so back to the old fashioned way
  template <class T>
  struct rebind
  {
    typedef expected<T, error_type> type;
  };

  using type_constructor = expected<holder, error_type>;

  // Constructors/Destructors/Assignments

  expected(const expected& rhs
    , BOOST_EXPECTED_REQUIRES( std::is_copy_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(rhs)
  {
  }

  expected(expected&& rhs
    , BOOST_EXPECTED_REQUIRES( std::is_move_constructible<error_type>::value)
  )
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_move_constructible<error_type>::value
  )
  : base_type(std::forward<expected>(rhs))
  {
  }

  BOOST_CONSTEXPR explicit expected(in_place_t) BOOST_NOEXCEPT
  : base_type(in_place2)
  {}
  BOOST_CONSTEXPR explicit expected(expect_t) BOOST_NOEXCEPT
  : base_type(in_place2)
  {}

  BOOST_EXPECTED_0_REQUIRES(std::is_default_constructible<error_type>::value)
  BOOST_CONSTEXPR expected()
      BOOST_NOEXCEPT_IF(
        std::is_nothrow_default_constructible<error_type>::value
      )
  : base_type()
  {}


  BOOST_EXPECTED_0_REQUIRES(std::is_copy_constructible<error_type>::value)
  expected(unexpected_type<error_type> const& e)
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(e)
  {}

  BOOST_EXPECTED_0_REQUIRES(std::is_move_constructible<error_type>::value)
  expected(unexpected_type<error_type> && e
  )
  BOOST_NOEXCEPT_IF(
    std::is_nothrow_move_constructible<error_type>::value
  )
  : base_type(std::forward<unexpected_type<error_type>>(e))
  {}

  template <class Err
  , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<error_type, Err>::value)
  >
  expected(unexpected_type<Err> const& e
  )
  BOOST_NOEXCEPT_IF((
    std::is_nothrow_constructible<error_type, Err>::value
  ))
  : base_type(e)
  {}

  template <class Err>
  expected(unexpected_type<Err> && e
//    , BOOST_EXPECTED_REQUIRES(std::is_copy_constructible<error_type, Err&&>::value)
  )
//  BOOST_NOEXCEPT_IF(
//    std::is_nothrow_constructible<error_type, Err&&>::value
//  )
  : base_type(std::forward<unexpected_type<Err>>(e))
  {}

  template <class... Args
#if !defined BOOST_EXPECTED_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  , BOOST_EXPECTED_T_REQUIRES(std::is_constructible<error_type, Args&...>::value)
#endif
  >
  expected(unexpect_t, Args&&... args
  )
  BOOST_NOEXCEPT_IF(
      std::is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(unexpected_type<error_type>(error_type(std::forward<Args>(args)...)))
  {}

  ~expected() = default;

  // Assignments
  expected& operator=(expected const& e)
  {
    this_type(e).swap(*this);
    return *this;
  }

  expected& operator=(expected&& e)
  {
    this_type(std::move(e)).swap(*this);
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
        error_type t = std::move(rhs.contained_err());
        ::new (errorptr()) error_type(t);
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
        using std::swap;
        swap(contained_err(), rhs.contained_err());
      }
    }
  }

  // Observers
  BOOST_CONSTEXPR bool valid() const BOOST_NOEXCEPT
  {
    return contained_has_value();
  }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
  BOOST_CONSTEXPR bool operator !() const BOOST_NOEXCEPT
  {
    return !valid();
  }
  BOOST_CONSTEXPR explicit operator bool() const BOOST_NOEXCEPT
  {
    return valid();
  }
#endif

  void value() const
  {
    if(!valid())
    {
      error_traits<error_type>::rethrow(contained_err());
    }
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
  error_type & error() BOOST_NOEXCEPT
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

  template <typename F>
  typename rebind<void>::type
  catch_all_void_void(F&& f)
  {
    typedef typename rebind<void>::type result_type;
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    try {
#endif
      f();
      return result_type(in_place_t{});
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename std::result_of<F()>::type
  catch_all_void_type(F&& f)
  {
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    //typedef typename std::result_of<F()>::type result_type;
    try {
#endif
      return f();
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<typename std::result_of<F()>::type>::type
  catch_all_void_etype(F&& f)
  {
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    //typedef typename rebind<typename std::result_of<F()>::type>::type result_type;
    try {
#endif
      return f();
#if defined BOOST_EXPECTED_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
//  template <typename F>
//  typename rebind<void>::type
//  catch_all_evoid_void(F&& f)
//  {
//    typedef typename rebind<void>::type result_type;
//    try {
//      f(std::move(*this));
//    } catch (...) {
//      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
//    }
//    return result_type(in_place_t{});
//  }
//  template <typename F>
//  typename std::result_of<F(expected)>::type
//  catch_all_evoid_type(F&& f)
//  {
//    //typedef typename std::result_of<F(expected)>::type result_type;
//    try {
//      return f(std::move(*this));
//    } catch (...) {
//      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
//    }
//  }
//  template <typename F>
//  typename rebind<typename std::result_of<F(expected)>::type>::type
//  catch_all_evoid_etype(F&& f)
//  {
//    //typedef typename rebind<typename std::result_of<F(expected)>::type>::type result_type;
//    try {
//      return f(std::move(*this));
//    } catch (...) {
//      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
//    }
//  }

  template <typename F>
  typename rebind<void>::type
  map(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(value_type)>::type, void>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return catch_all_void_void(std::forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<void>::type result_type;
    return (valid()
        ? catch_all_void_void(std::forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  typename rebind<typename std::result_of<F(value_type)>::type>::type
  map(F&& f,
    BOOST_EXPECTED_REQUIRES(!std::is_same<typename std::result_of<F(value_type)>::type, void>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return catch_all_void_etype(std::forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<typename std::result_of<F(value_type)>::type>::type result_type;
    return (valid()
        ? catch_all_void_etype(std::forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

  // bind factory

//  template <typename F>
//  BOOST_CONSTEXPR typename rebind<void>::type bind(F&& f,
//    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F()>::type, void>::value)) const
//  {
//    typedef typename rebind<void>::type result_type;
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        f();
//        return result_type(in_place_t{});
//    }
//    return get_unexpected();
//#else
//    return ( valid()
//        ? ( f(), result_type(in_place_t{}) )
//        :  result_type(get_unexpected())
//        );
//#endif
//  }
//
//  template <typename F>
//  typename rebind<typename std::result_of<F()>::type>::type
//  bind(F&& f,
//    BOOST_EXPECTED_REQUIRES( ! std::is_same<typename std::result_of<F()>::type, void>::value
//        && ! boost::is_expected<typename std::result_of<F(value_type)>::type>::value
//        ) )
//  {
//    typedef typename rebind<typename std::result_of<F()>::type>::type result_type;
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        return result_type(f());
//    }
//    return get_unexpected();
//#else
//    return ( valid()
//        ? result_type(f())
//        :  result_type(get_unexpected())
//        );
//#endif
//  }

  template <typename F>
  typename std::result_of<F()>::type
  bind(F&& f,
    BOOST_EXPECTED_REQUIRES( boost::is_expected<typename std::result_of<F(value_type)>::type>::value
        ) )
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return f();
    }
    return get_unexpected();
#else
    typedef typename std::result_of<F()>::type result_type;
    return ( valid()
        ? f()
        :  result_type(get_unexpected())
        );
#endif
  }

  template <typename F>
  typename rebind<void>::type
  then(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(expected)>::type, void>::value))
  {
    typedef typename rebind<void>::type result_type;
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    f(std::move(*this));
    return result_type(in_place_t{});
#else
    return ( f(std::move(*this)), result_type(in_place_t{}) );
#endif
  }

  // then factory
  template <typename F>
  typename rebind<typename std::result_of<F(expected)>::type>::type
  then(F&& f,
    BOOST_EXPECTED_REQUIRES(!boost::is_expected<typename std::result_of<F(expected)>::type>::value
        ))
  {
    typedef typename rebind<typename std::result_of<F(expected)>::type>::type result_type;
    return result_type(f(std::move(*this)));
  }

  template <typename F>
  typename std::result_of<F(expected)>::type
  then(F&& f,
    BOOST_EXPECTED_REQUIRES(!std::is_same<typename std::result_of<F(expected)>::type, void>::value
        && boost::is_expected<typename std::result_of<F(expected)>::type>::value
        )
    )
  {
    return f(std::move(*this));
  }

  // catch_error factory

  template <typename F>
  this_type catch_error(F&& f,
    BOOST_EXPECTED_REQUIRES(std::is_same<typename std::result_of<F(error_type)>::type, value_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
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
  this_type catch_error(F&& f,
      BOOST_EXPECTED_REQUIRES(! std::is_same<typename std::result_of<F(error_type)>::type, value_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
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
  this_type catch_exception(F&& f,
    BOOST_EXPECTED_REQUIRES(
        std::is_same<typename std::result_of<F(Ex &)>::type, this_type>::value
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
  this_type catch_exception(F&& f,
    BOOST_EXPECTED_REQUIRES(
        std::is_same<typename std::result_of<F(Ex &)>::type, value_type>::value
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
BOOST_CONSTEXPR bool operator==(const expected<T,E>& x, const expected<T,E>& y)
{
  return (x && y)
    ? *x == *y
    : (!x && !y)
      ?  x.get_unexpected() == y.get_unexpected()
      : false;
}

template <class E>
BOOST_CONSTEXPR bool operator==(const expected<void, E>& x, const expected<void, E>& y)
{
  return (x && y)
    ? true
    : (!x && !y)
      ?  x.get_unexpected() == y.get_unexpected()
      : false;
}

template <class T, class E>
BOOST_CONSTEXPR bool operator!=(const expected<T,E>& x, const expected<T,E>& y)
{
  return !(x == y);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator<(const expected<T,E>& x, const expected<T,E>& y)
{
  return (x)
    ? (y) ? *x < *y : false
    : (y) ? true : x.get_unexpected() < y.get_unexpected();
}

template <class E>
BOOST_CONSTEXPR bool operator<(const expected<void, E>& x, const expected<void, E>& y)
{
  return (x)
    ? (y) ? false : false
    : (y) ? true : x.get_unexpected() < y.get_unexpected();
}

template <class T, class E>
BOOST_CONSTEXPR bool operator>(const expected<T,E>& x, const expected<T,E>& y)
{
  return (y < x);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator<=(const expected<T,E>& x, const expected<T,E>& y)
{
  return !(y < x);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator>=(const expected<T,E>& x, const expected<T,E>& y)
{
  return !(x < y);
}

// Relational operators with T
template <class T, class E>
BOOST_CONSTEXPR bool operator==(const expected<T,E>& x, const T& v)
{
  return (x) ? *x == v :  false;
}
template <class E>
BOOST_CONSTEXPR bool operator==(const E& v, const expected<E,void>& x)
{
  return x == v;
}

template <class T, class E>
BOOST_CONSTEXPR bool operator!=(const expected<T,E>& x, const T& v)
{
  return ! (x == v);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator!=(const T& v, const expected<T,E>& x)
{
  return x != v;
}

template <class T, class E>
BOOST_CONSTEXPR bool operator<(const expected<T,E>& x, const T& v)
{
  return (x) ? (*x < v) : true ;
}
template <class T, class E>
BOOST_CONSTEXPR bool operator<(const T& v, const expected<T,E>& x)
{
  return (x) ? (v < x) : false ;
}

template <class T, class E>
BOOST_CONSTEXPR bool operator>(const expected<T,E>& x, const T& v)
{
  return v < x;
}
template <class T, class E>
BOOST_CONSTEXPR bool operator>(const T& v, const expected<T,E>& x)
{
  return x < v;
}

template <class T, class E>
BOOST_CONSTEXPR bool operator<=(const expected<T,E>& x, const T& v)
{
  return ! (v < x);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator<=(const T& v, const expected<T,E>& x)
{
  return ! (x < v);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator>=(const expected<T,E>& x, const T& v)
{
  return ! (x < v);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator>=(const T& v, const expected<T,E>& x)
{
  return ! (v < x);
}

// Relational operators with unexpected_type<E>
template <class T, class E>
BOOST_CONSTEXPR bool operator==(const expected<T,E>& x, const unexpected_type<E>& e)
{
  return (!x) ? x.get_unexpected() == e :  false;
}
template <class T, class E>
BOOST_CONSTEXPR bool operator==(const unexpected_type<E>& e, const expected<T,E>& x)
{
  return (x == e);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator!=(const expected<T,E>& x, const unexpected_type<E>& e)
{
  return ! (x == e);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator!=(const unexpected_type<E>& e, const expected<T,E>& x)
{
  return ! (x == e);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator<(const expected<T,E>& x, const unexpected_type<E>& e)
{
  return (!x) ? (x.get_unexpected() < e) : false ;
}
template <class T, class E>
BOOST_CONSTEXPR bool operator<(const unexpected_type<E>& e, const expected<T,E>& x)
{
  return (!x) ? (e < x.get_unexpected()) : true ;
}

template <class T, class E>
BOOST_CONSTEXPR bool operator>(const expected<T,E>& x, const unexpected_type<E>& e)
{
  return (e <  x);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator>(const unexpected_type<E>& e, const expected<T,E>& x)
{
  return (x <  e);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator<=(const expected<T,E>& x, const unexpected_type<E>& e)
{
  return ! (e < x);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator<=(const unexpected_type<E>& e, const expected<T,E>& x)
{
  return ! (x < e);
}

template <class T, class E>
BOOST_CONSTEXPR bool operator>=(const expected<T,E>& x, const unexpected_type<E>& e)
{
  return ! (e > x);
}
template <class T, class E>
BOOST_CONSTEXPR bool operator>=(const unexpected_type<E>& e, const expected<T,E>& x)
{
  return ! (x > e);
}

// Specialized algorithms
template <class T, class E>
void swap(expected<T,E>& x, expected<T,E>& y) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(x.swap(y)))
{
  x.swap(y);
}

// Factories

template <typename T>
BOOST_CONSTEXPR expected<decay_t<T>, std::exception_ptr> make_expected(T&& v)
{
  return expected<decay_t<T>, std::exception_ptr>(constexpr_forward<T>(v));
}

BOOST_FORCEINLINE expected<void, std::exception_ptr> make_expected()
{
  return expected<void, std::exception_ptr>(in_place2);
}

template <typename T>
BOOST_FORCEINLINE expected<T, std::exception_ptr> make_expected_from_current_exception() BOOST_NOEXCEPT
{
  return expected<T, std::exception_ptr>(make_unexpected_from_current_exception());
}

template <typename T>
BOOST_FORCEINLINE expected<T, std::exception_ptr> make_expected_from_exception(std::exception_ptr e) BOOST_NOEXCEPT
{
  return expected<T, std::exception_ptr>(unexpected_type<>(e));
}

template <class T, class E>
BOOST_FORCEINLINE expected<T, std::exception_ptr> make_expected_from_exception(E&& e) BOOST_NOEXCEPT
{
  return expected<T, std::exception_ptr>(unexpected_type<>(constexpr_forward<E>(e)));
}

template <class T, class E>
BOOST_FORCEINLINE BOOST_CONSTEXPR
expected<T, decay_t<E>> make_expected_from_error(E&& e) BOOST_NOEXCEPT
{
  return expected<T, decay_t<E>>(make_unexpected(constexpr_forward<E>(e)));
}

template <class T, class E, class U>
BOOST_FORCEINLINE BOOST_CONSTEXPR
expected<T, E> make_expected_from_error(U&& u) BOOST_NOEXCEPT
{
  return expected<T, E>(make_unexpected(E(constexpr_forward<U>(u))));
}

template <typename F>
expected<typename std::result_of<F()>::type, std::exception_ptr>
BOOST_FORCEINLINE make_expected_from_call(F funct
  , BOOST_EXPECTED_REQUIRES( ! std::is_same<typename std::result_of<F()>::type, void>::value)
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
inline expected<void, std::exception_ptr>
make_expected_from_call(F funct
  , BOOST_EXPECTED_REQUIRES( std::is_same<typename std::result_of<F()>::type, void>::value)
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

template <class T, class E>
BOOST_FORCEINLINE BOOST_CONSTEXPR unexpected_type<E> make_unexpected(expected<T,E>& ex)
{
  return unexpected_type<E>(ex.error());
}

namespace expected_detail
{

  // Factories

  template <class T, class E>
  inline BOOST_CONSTEXPR expected<T,E> unwrap(expected<expected<T,E>, E > const& ee)
  {
     return (ee) ? *ee : ee.get_unexpected();
  }
  template <class T, class E>
  inline BOOST_CONSTEXPR expected<T,E> unwrap(expected<expected<T,E>, E >&& ee)
  {
    return (ee) ? std::move(*ee) : ee.get_unexpected();
  }
  template <class T, class E>
  inline BOOST_CONSTEXPR expected<T,E> unwrap(expected<T,E> const& e)
  {
    return e;
  }
  template <class T, class E>
  inline BOOST_CONSTEXPR expected<T,E> unwrap(expected<T,E> && e)
  {
    return std::move(e);
  }

} // namespace expected_detail

#if ! defined BOOST_EXPECTED_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  template <class T, class E>
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected<T,E>>
  expected<T,E>::unwrap() const&
  {
    return expected_detail::unwrap(*this);
  }
  template <class T, class E>
  inline BOOST_EXPECTED_CONSTEXPR_IF_MOVE_ACCESSORS expected_detail::unwrap_result_type_t<expected<T,E>>
  expected<T,E>::unwrap() &&
  {
    return expected_detail::unwrap(*this);
  }
#else
  template <class T, class E>
  inline BOOST_CONSTEXPR expected_detail::unwrap_result_type_t<expected<T,E>>
  expected<T,E>::unwrap() const
  {
    return expected_detail::unwrap(*this);
  }
#endif
} // namespace boost


#endif // BOOST_EXPECTED_EXPECTED_HPP
