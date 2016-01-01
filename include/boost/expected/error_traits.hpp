// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2015 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_ERROR_TRAITS_HPP
#define BOOST_EXPECTED_ERROR_TRAITS_HPP

#include <boost/expected/bad_expected_access.hpp>
#include <boost/exception_ptr.hpp>
#include <exception>
#include <system_error>

namespace boost {
  template <class Error>
  struct error_traits
  {
    template <class Exception>
    static Error make_error(Exception const& e)
    {
      return Error{e};
    }
    static Error make_error_from_current_exception()
    {
      try {
        throw;
      } catch (std::exception & e) {
        return make_error(e);
      } catch (...) {
        return Error{};
      }
    }
    static void rethrow(Error const& e)
    {
      throw bad_expected_access<Error>{e};
    }
  };

  template <>
  struct error_traits<exception_ptr>
  {
    template <class Exception>
    static exception_ptr make_error(Exception const&e)
    {
      return copy_exception(e);
    }
    static exception_ptr make_error_from_current_exception()
    {
      return current_exception();
    }
    static void rethrow(exception_ptr const& e)
    {
      rethrow_exception(e);
    }
  };

  template <>
  struct error_traits<std::exception_ptr>
  {
    template <class Exception>
    static std::exception_ptr make_error(Exception const&e)
    {
      return std::make_exception_ptr(e);
    }
    static std::exception_ptr make_error_from_current_exception()
    {
      return std::current_exception();
    }
    static void rethrow(std::exception_ptr const& e)
    {
      std::rethrow_exception(e);
    }
  };

  template <>
  struct error_traits<std::error_code>
  {
    template <class Exception>
    static std::error_code make_error(std::system_error const&e)
    {
      return e.code();
    }
    template <class Exception>
    // requires is_base_of<std::system_error, Exception>
    static std::error_code make_error(Exception const&e)
    {
      return e.code();
    }

    static std::error_code make_error_from_current_exception()
    {
      try {
        throw;
      } catch (std::system_error & e) {
        return make_error(e);
      } catch (...) {
        return std::error_code();
      }
    }
    static void rethrow(std::error_code const& e)
    {
      throw std::system_error(e);
    }
  };

}
#endif
