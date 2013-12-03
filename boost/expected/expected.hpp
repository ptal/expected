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
#include <boost/utility/swap.hpp>
#include <utility>
#include <initializer_list>

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
  
  static error_type catch_exception()
  {
    throw;
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
  static error_type make_exceptional(E const& e)
  {
    return boost::copy_exception(e);
  }
  
  static error_type catch_exception()
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
{
  typedef std::exception_ptr error_type;

  template <class E>
  static error_type make_exceptional(E const& e)
  {
    return std::make_exception_ptr(e);
  }
  
  static error_type catch_exception()
  {
    return std::current_exception();
  }
  
  static void bad_access(const error_type &e)
  {
    std::rethrow_exception(e);
  }
};
#endif

struct exceptional_tag {};
BOOST_CONSTEXPR_OR_CONST exceptional_tag exceptional = {};

struct emplace_tag {};
BOOST_CONSTEXPR_OR_CONST emplace_tag emplace = {};

template <class V, class E>
class expected;

#if ! defined BOOST_NO_CXX11_HDR_EXCEPTION  && ! defined BOOST_NO_CXX11_RVALUE_REFERENCES
  template <typename ValueType, typename ExceptionalType=std::exception_ptr>
#else
  template <typename ValueType, typename ExceptionalType=boost::exception_ptr>
#endif
  class expected
  {
  public:
    typedef ValueType value_type;
    typedef ExceptionalType error_type;
    typedef expected_traits<error_type> traits_type;

  private:
    typedef expected<value_type, error_type> this_type;

    // Static asserts.
    typedef boost::is_same<value_type, exceptional_tag> is_same_value_exceptional_tag;
    typedef boost::is_same<value_type, emplace_tag> is_same_value_emplace_tag;
    typedef boost::is_same<error_type, exceptional_tag> is_same_exceptional_exceptional_tag;
    typedef boost::is_same<error_type, emplace_tag> is_same_exceptional_emplace_tag;
    BOOST_STATIC_ASSERT_MSG( !is_same_value_exceptional_tag::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_value_emplace_tag::value, "bad ValueType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_exceptional_exceptional_tag::value, "bad ExceptionalType" );
    BOOST_STATIC_ASSERT_MSG( !is_same_exceptional_emplace_tag::value, "bad ExceptionalType" );

    // C++03 movable support
    BOOST_COPYABLE_AND_MOVABLE(this_type)

  private:
    union {
      error_type error_;
      value_type value;
    };
    bool has_value;

  public:

    // About noexcept specification:
    //  has_nothrow_move_constructor is not yet into Boost.TypeTraits.

    // Constructors/Destructors/Assignments

#ifdef BOOST_EXPECTED_USE_DEFAULT_CONSTRUCTOR
    template <typename U = ValueType, class = typename std::enable_if<std::is_default_constructible<U>::value>::type>
    BOOST_CONSTEXPR expected()
    : value()
    , has_value(true)
    {}
#endif

    BOOST_CONSTEXPR expected(const value_type& rhs)
    : value(rhs)
    , has_value(true)
    {}
    
    BOOST_CONSTEXPR expected(BOOST_RV_REF(value_type) rhs)
    //BOOST_NOEXCEPT_IF(
    //  has_nothrow_move_constructor<value_type>::value
    //)
    : value(boost::move(rhs))
    , has_value(true)
    {}

    expected(const expected& rhs) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<value_type>::value && 
      has_nothrow_copy_constructor<error_type>::value
    )
    : has_value(rhs.has_value)
    {
      if (has_value)
      {
        ::new (&value) value_type(rhs.value);
      }
      else
      {
        ::new (&error_) error_type(rhs.error_);
      }
    }

    expected(BOOST_RV_REF(expected) rhs)
    //BOOST_NOEXCEPT_IF(
    //  has_nothrow_move_constructor<value_type>::value && 
    //  has_nothrow_move_constructor<error_type>::value
    //)
    : has_value(rhs.has_value)
    {
      if (has_value)
      {
        ::new (&value) value_type(boost::move(rhs.value));
      }
      else
      {
        ::new (&error_) error_type(boost::move(rhs.error_));
      }
    }

    expected(exceptional_tag, error_type const& e) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<error_type>::value
    )
    : error_(e)
    , has_value(false)
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    expected(exceptional_tag, E const& e)
    : error_(traits_type::make_exceptional(e))
    , has_value(false)
    {}

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <class... Args>
    BOOST_CONSTEXPR explicit expected(emplace_tag, BOOST_FWD_REF(Args)... args)
    : value(boost::forward<Args>(args)...)
    , has_value(true)
    {}
#else
#define EXPECTED_EMPLACE_CONSTRUCTOR(z, n, unused)                            \
    template <BOOST_PP_ENUM_PARAMS(n, class Arg)>                             \
    explicit expected(emplace_tag,                                            \
      BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_REF_ARG, ~))                          \
    : value(BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_PARAM, ~))                      \
    , has_value(true)                                                         \
    {}

BOOST_PP_REPEAT_FROM_TO(1, BOOST_EXPECTED_EMPLACE_MAX_ARGS, EXPECTED_EMPLACE_CONSTRUCTOR, ~)

#undef EXPECTED_EMPLACE_CONSTRUCTOR
#endif

    expected(exceptional_tag)
    : error_(traits_type::catch_exception())
    , has_value(false)
    {}

    ~expected()
    {
      if (valid()) value.~value_type();
      else error_.~error_type();
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
    expected& emplace(BOOST_FWD_REF(Args)... args)
    {
      // Why emplace doesn't work (instead of emplace_tag()) ?
      this_type(emplace_tag(), boost::forward<Args>(args)...).swap(*this);
      return *this;
    }
#else
#define EXPECTED_EMPLACE(z, n, unused)                                        \
    template <BOOST_PP_ENUM_PARAMS(n, class Arg)>                             \
    expected& emplace(BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_REF_ARG, ~))          \
    {                                                                         \
      this_type(emplace_tag(), BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_PARAM, ~))   \
      .swap(*this);                                                           \
      return *this;                                                           \
    }

BOOST_PP_REPEAT_FROM_TO(1, BOOST_EXPECTED_EMPLACE_MAX_ARGS, EXPECTED_EMPLACE, ~)

#undef EXPECTED_EMPLACE
#endif

    // Modifiers
    void swap(expected& rhs)
    {
      if (has_value)
      {
        if (rhs.has_value)
        {
          boost::swap(value, rhs.value);
        }
        else
        {
          error_type t = boost::move(rhs.error_);
          new (&rhs.value) value_type(boost::move(value));
          new (&error_) error_type(t);
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
          boost::swap(error_, rhs.error_);
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
      if (!valid()) traits_type::bad_access(error_);
      return value;
    }

    value_type& get()
    {
      if (!valid()) traits_type::bad_access(error_);
      return value;
    }

    BOOST_CONSTEXPR value_type const& operator*() const BOOST_NOEXCEPT
    {
      return value;
    }

    value_type& operator*() BOOST_NOEXCEPT
    {
      return value;
    }

    BOOST_CONSTEXPR error_type const& error() const BOOST_NOEXCEPT
    {
      return error_;
    }

    error_type& error() BOOST_NOEXCEPT
    {
      return error_;
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F(value_type)>::type, void>,
      expected<void, error_type>
    >::type
    then(BOOST_RV_REF(F) f) const
    {
      typedef expected<void, error_type> result_type;
      if(valid())
      {
        try
        {
          f(value);
          return result_type();
        }
        catch(...)
        {
          return result_type(exceptional);
        }
      }
      return result_type(exceptional, error_);
    }

    template <typename F>
    typename boost::disable_if<
      boost::is_same<typename result_of<F(value_type)>::type, void>,
      expected<typename result_of<F(value_type)>::type, error_type>
    >::type
    then(BOOST_RV_REF(F) f) const
    {
      typedef expected<typename result_of<F(value_type)>::type, error_type> result_type;
      if(valid())
      {
        try
        {
          return result_type(f(value));
        }
        catch(...)
        {
          return result_type(exceptional);
        }
      }
      return result_type(exceptional, error_);
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F(error_type)>::type, value_type>,
      this_type
    >::type
    recover(BOOST_RV_REF(F) f) const
    {
      if(!valid())
      {
        try
        {
          return this_type(f(error_));
        }
        catch(...)
        {
          return this_type(exceptional);
        }
      }
      return this_type(value);
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F(error_type)>::type, this_type>,
      this_type
    >::type
    recover(BOOST_RV_REF(F) f) const
    {
      if(!valid())
      {
        try
        {
          return f(error_);
        }
        catch(...)
        {
          return this_type(exceptional);
        }
      }
      return this_type(value);
    }
  };

  template <typename ExceptionalType>
  class expected<void, ExceptionalType>
  {
  public:
    typedef void value_type;
    typedef ExceptionalType error_type;
    typedef expected_traits<error_type> traits_type;

  private:
    typedef expected<value_type, error_type> this_type;

    // C++03 movable support
    BOOST_COPYABLE_AND_MOVABLE(this_type)

  private:
    error_type error_;
    bool has_value;

  public:
    // About noexcept specification:
    //  has_nothrow_move_constructor is not yet into Boost.TypeTraits.

    // Constructors/Destructors/Assignments

    expected(const expected& rhs) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<error_type>::value
    )
    : has_value(rhs.has_value)
    {
      if (!has_value)
      {
        ::new (&error_) error_type(rhs.error_);
      }
    }

    expected(BOOST_RV_REF(expected) rhs)
    : has_value(rhs.has_value)
    {
      if (!has_value)
      {
        ::new (&error_) error_type(boost::move(rhs.error_));
      }
    }

    expected(exceptional_tag, error_type const& e) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<error_type>::value
    )
    : error_(e)
    , has_value(false)
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    expected(exceptional_tag, E const& e)
    : error_(traits_type::make_exceptional(e))
    , has_value(false)
    {}

    expected()
    : has_value(true)
    {}

    expected(exceptional_tag)
    : error_(traits_type::catch_exception())
    , has_value(false)
    {}

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

    // Modifiers
    void swap(expected& rhs)
    {
      if (has_value)
      {
        if (!rhs.has_value)
        {
          new (&error_) error_type(boost::move(rhs.error_));
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
          boost::swap(error_, rhs.error_);
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

    void get() const
    {
      if (!valid()) traits_type::bad_access(error_);
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F()>::type, void>,
      this_type
    >::type
    then(BOOST_RV_REF(F) f) const
    {
      if(valid())
      {
        try
        {
          f();
          return this_type();
        }
        catch(...)
        {
          return this_type(exceptional);
        }
      }
      return this_type(exceptional, error_);
    }

    template <typename F>
    typename boost::disable_if<
      boost::is_same<typename result_of<F()>::type, void>,
      expected<typename result_of<F()>::type, error_type>
    >::type
    then(BOOST_RV_REF(F) f) const
    {
      typedef expected<typename result_of<F()>::type, error_type> result_type;
      if(valid())
      {
        try
        {
          return result_type(f());
        }
        catch(...)
        {
          return result_type(exceptional);
        }
      }
      return result_type(exceptional, error_);
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F(error_type)>::type, value_type>,
      this_type
    >::type
    recover(BOOST_RV_REF(F) f) const
    {
      if(!valid())
      {
        try
        {
          f(error_);
        }
        catch(...)
        {
          return this_type(exceptional);
        }
      }
      return this_type();
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F(error_type)>::type, this_type>,
      this_type
    >::type
    recover(BOOST_RV_REF(F) f) const
    {
      if(!valid())
      {
        try
        {
          return f(error_);
        }
        catch(...)
        {
          return this_type(exceptional);
        }
      }
      return this_type();
    }
  };

  // Specialized algorithms
  template <class T>
  void swap(expected<T>& x, expected<T>& y) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(x.swap(y)))
  {
    x.swap(y);
  }

#if ! defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
  // Factories
  template<typename T, typename E, typename... Args>
  expected<T,E> make_expected(BOOST_FWD_REF(Args)... args)
  {
    return expected<T,E>(emplace, boost::forward<Args>(args)...);
  }

  template<typename T, typename... Args>
  expected<T> make_expected(BOOST_FWD_REF(Args)... args)
  {
    return expected<T>(emplace, boost::forward<Args>(args)...);
  }
#else
#define MAKE_EXPECTED_EMPLACE(z, n, unused)                                   \
    template<typename T, typename E, BOOST_PP_ENUM_PARAMS(n, class Arg)>      \
    expected<T,E> make_expected(BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_REF_ARG , ~))\
    {                                                                         \
      return expected<T,E>(emplace,                                           \
        BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_PARAM, ~));                        \
    }

BOOST_PP_REPEAT_FROM_TO(1, BOOST_EXPECTED_EMPLACE_MAX_ARGS, MAKE_EXPECTED_EMPLACE, ~)
#undef MAKE_EXPECTED_EMPLACE

#define MAKE_EXPECTED_EMPLACE(z, n, unused)                                 \
    template<typename T, BOOST_PP_ENUM_PARAMS(n, class Arg)>                \
    expected<T> make_expected(BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_REF_ARG, ~)) \
    {                                                                       \
      return expected<T>(emplace,                                           \
        BOOST_PP_REPEAT(n, MAKE_BOOST_FWD_PARAM, ~));                      \
    }

BOOST_PP_REPEAT_FROM_TO(1, BOOST_EXPECTED_EMPLACE_MAX_ARGS, MAKE_EXPECTED_EMPLACE, ~)
#undef MAKE_EXPECTED_EMPLACE
#endif

  template <typename T>
  expected<T> make_exceptional_expected() BOOST_NOEXCEPT
  {
    return expected<T>(exceptional);
  }

  template <typename T, typename E>
  expected<T, E> make_exceptional_expected()
  {
    return expected<T, E>(exceptional);
  }

  template <typename T>
#ifdef BOOST_EXPECTED_USE_STD_EXCEPTION_PTR
  expected<T> make_exceptional_expected(std::exception_ptr const& e) BOOST_NOEXCEPT
#else
  expected<T> make_exceptional_expected(boost::exception_ptr const& e) BOOST_NOEXCEPT
#endif
  {
    return expected<T>(exceptional, e);
  }

  template <typename F>
  typename boost::enable_if<
    boost::is_same<typename result_of<F()>::type, void>,
    expected<typename result_of<F()>::type>
  >::type
  make_noexcept_expected(BOOST_RV_REF(F) f) BOOST_NOEXCEPT
  {
    typedef typename boost::result_of<F()>::type T;
    try
    {
      f();
      return expected<T>();
    }
    catch (...)
    {
      return make_exceptional_expected<T>();
    }
  }

  template <typename F>
  typename boost::disable_if<
    boost::is_same<typename result_of<F()>::type, void>,
    expected<typename result_of<F()>::type>
  >::type
  make_noexcept_expected(BOOST_RV_REF(F) f) BOOST_NOEXCEPT
  {
    typedef typename boost::result_of<F()>::type T;
    try
    {
      return expected<T>(f());
    }
    catch (...)
    {
      return make_exceptional_expected<T>();
    }
  }

  template <typename E, typename F>
  typename boost::enable_if<
    boost::is_same<typename result_of<F()>::type, void>,
    expected<typename result_of<F()>::type, E>
  >::type
  make_noexcept_expected(BOOST_RV_REF(F) f)
  {
    typedef typename boost::result_of<F()>::type T;
    try
    {
      f();
      return expected<T, E>();
    }
    catch (...)
    {
      return make_exceptional_expected<T, E>();
    }
  }

  template <typename E, typename F>
  typename boost::disable_if<
    boost::is_same<typename result_of<F()>::type, void>,
    expected<typename result_of<F()>::type, E>
  >::type
  make_noexcept_expected(BOOST_RV_REF(F) f)
  {
    typedef typename boost::result_of<F()>::type T;
    try
    {
      return expected<T, E>(f());
    }
    catch (...)
    {
      return make_exceptional_expected<T, E>();
    }
  }
} // namespace boost

#if defined BOOST_NO_CXX11_VARIADIC_TEMPLATES
  #undef MAKE_BOOST_FWD_REF_ARG
  #undef MAKE_BOOST_FWD_PARAM
#endif

#endif // BOOST_EXPECTED_HPP
