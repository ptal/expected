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

// define BOOST_USE_STD_EXCEPTION_PTR to enable the use of the standard exception library.

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
      throw;
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

#ifdef BOOST_USE_STD_EXCEPTION_PTR
  template <>
  struct exceptional_traits<std::exception_ptr>
  {
    typedef std::exception_ptr exceptional_type;

    template <class E>
    static exceptional_type make_exceptional(E const& e)
    {
      return std::make_exception_ptr(e);
    }
    
    static exceptional_type catch_exception()
    {
      return std::current_exception();
    }
    
    static void bad_access(const exceptional_type &e)
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

#ifdef BOOST_USE_STD_EXCEPTION_PTR
  template <typename ValueType, typename ExceptionalType=std::exception_ptr>
#else
  template <typename ValueType, typename ExceptionalType=boost::exception_ptr>
#endif
  class expected
  {
  public:
    typedef ValueType value_type;
    typedef ExceptionalType exceptional_type;
    typedef exceptional_traits<exceptional_type> traits_type;

  private:
    typedef expected<value_type, exceptional_type> this_type;

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

    // About noexcept specification:
    //  has_nothrow_move_constructor is not yet into Boost.TypeTraits.

    // Constructors/Destructors/Assignments

    BOOST_CONSTEXPR expected(const value_type& rhs) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(value(rhs)))
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
      has_nothrow_copy_constructor<exceptional_type>::value
    )
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

    expected(BOOST_RV_REF(expected) rhs)
    //BOOST_NOEXCEPT_IF(
    //  has_nothrow_move_constructor<value_type>::value && 
    //  has_nothrow_move_constructor<exceptional_type>::value
    //)
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

    expected(exceptional_tag, exceptional_type const& e) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<exceptional_type>::value
    )
    : error(e)
    , has_value(false)
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    expected(exceptional_tag, E const& e)
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

    expected(exceptional_tag)
    : error(traits_type::catch_exception())
    , has_value(false)
    {}

    ~expected()
    {
      if (valid()) value.~value_type();
      else error.~exceptional_type();
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
      // Why emplace doesn't work (instead of emplace_tag()) ?
      this_type(emplace_tag(), boost::forward<Args>(args)...).swap(*this);
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
    explicit operator bool() const BOOST_NOEXCEPT
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

    BOOST_CONSTEXPR value_type const& operator*() const BOOST_NOEXCEPT
    {
      return value;
    }

    value_type& operator*() BOOST_NOEXCEPT
    {
      return value;
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F(value_type)>::type, void>,
      expected<typename result_of<F(value_type)>::type, exceptional_type>
    >::type
    then(const F& f) const
    {
      typedef expected<typename result_of<F(value_type)>::type, exceptional_type> result_type;
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
      return result_type(exceptional, error);
    }

    template <typename F>
    typename boost::disable_if<
      boost::is_same<typename result_of<F(value_type)>::type, void>,
      expected<typename result_of<F(value_type)>::type, exceptional_type>
    >::type
    then(const F& f) const
    {
      typedef expected<typename result_of<F(value_type)>::type, exceptional_type> result_type;
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
      return result_type(exceptional, error);
    }
  };

  template <typename ExceptionalType>
  class expected<void, ExceptionalType>
  {
  public:
    typedef void value_type;
    typedef ExceptionalType exceptional_type;
    typedef exceptional_traits<exceptional_type> traits_type;

  private:
    typedef expected<value_type, exceptional_type> this_type;

    // C++03 movable support
    BOOST_COPYABLE_AND_MOVABLE(this_type)

  private:
    exceptional_type error;
    bool has_value;

  public:
    // About noexcept specification:
    //  has_nothrow_move_constructor is not yet into Boost.TypeTraits.

    // Constructors/Destructors/Assignments

    expected(const expected& rhs) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<exceptional_type>::value
    )
    : has_value(rhs.has_value)
    {
      if (!has_value)
      {
        ::new (&error) exceptional_type(rhs.error);
      }
    }

    expected(BOOST_RV_REF(expected) rhs)
    : has_value(rhs.has_value)
    {
      if (!has_value)
      {
        ::new (&error) exceptional_type(boost::move(rhs.error));
      }
    }

    expected(exceptional_tag, exceptional_type const& e) 
    BOOST_NOEXCEPT_IF(
      has_nothrow_copy_constructor<exceptional_type>::value
    )
    : error(e)
    , has_value(false)
    {}

    // Requires  typeid(e) == typeid(E)
    template <class E>
    expected(exceptional_tag, E const& e)
    : error(traits_type::make_exceptional(e))
    , has_value(false)
    {}

    expected()
    : has_value(true)
    {}

    expected(exceptional_tag)
    : error(traits_type::catch_exception())
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
          new (&error) exceptional_type(boost::move(rhs.error));
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
    explicit operator bool() const BOOST_NOEXCEPT
    {
      return valid();
    }
#endif

    void get() const
    {
      if (!valid()) traits_type::bad_access(error);
    }

    template <typename F>
    typename boost::enable_if<
      boost::is_same<typename result_of<F()>::type, void>,
      expected<typename result_of<F()>::type, exceptional_type>
    >::type
    then(const F& f) const
    {
      typedef expected<typename result_of<F()>::type, exceptional_type> result_type;
      if(valid())
      {
        try
        {
          f();
          return result_type();
        }
        catch(...)
        {
          return result_type(exceptional);
        }
      }
      return result_type(exceptional, error);
    }

    template <typename F>
    typename boost::disable_if<
      boost::is_same<typename result_of<F()>::type, void>,
      expected<typename result_of<F()>::type, exceptional_type>
    >::type
    then(const F& f) const
    {
      typedef expected<typename result_of<F()>::type, exceptional_type> result_type;
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
      return result_type(exceptional, error);
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
  expected<T,E> make_expected(Args&&... args)
  {
    return expected<T,E>(emplace, boost::forward<Args>(args)...);
  }

  template<typename T, typename... Args>
  expected<T> make_expected(Args&&... args)
  {
    return expected<T>(emplace, boost::forward<Args>(args)...);
  }
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

#ifdef BOOST_USE_STD_EXCEPTION_PTR
  template <typename T>
  expected<T> make_exceptional_expected(std::exception_ptr const& e) BOOST_NOEXCEPT
  {
    return expected<T>(exceptional, e);
  }
#endif

  template <typename T>
  expected<T> make_exceptional_expected(boost::exception_ptr const& e) BOOST_NOEXCEPT
  {
    return expected<T>(exceptional, e);
  }

  template <typename F>
  typename boost::enable_if<
    boost::is_same<typename result_of<F()>::type, void>,
    expected<typename result_of<F()>::type>
  >::type
  make_noexcept_expected(F f) BOOST_NOEXCEPT
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
  make_noexcept_expected(F f) BOOST_NOEXCEPT
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
  make_noexcept_expected(F f)
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
  make_noexcept_expected(F f)
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

#endif // BOOST_EXPECTED_HPP
