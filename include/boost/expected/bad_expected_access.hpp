// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2015 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_BAD_EXPECTED_ACCESS_HPP
#define BOOST_EXPECTED_BAD_EXPECTED_ACCESS_HPP

#include <stdexcept>

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


}
#endif
