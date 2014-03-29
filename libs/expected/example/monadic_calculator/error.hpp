// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Pierre Talbot

#ifndef MOCA_ERROR_HPP
#define MOCA_ERROR_HPP

#include <system_error>
#include <array>

namespace moca{
enum error
{
  integer_overflow,
  unknown_symbol,
  operation_overflow,
  division_by_zero,
  expected_operand,
  expected_operator,
  num_error
};
} // namespace moca

namespace std{

template<>
struct is_error_condition_enum<moca::error>
{
  static const bool value = true;
};

template<>
struct is_error_code_enum<moca::error>
{
  static const bool value = true;
};

} // namespace std

namespace moca{

class error_category : public std::error_category
{
  static const std::array<std::string, num_error> error_messages;
public:
  const char* name() const noexcept;
  std::string message(int ev) const;
};

std::error_condition make_error_condition(error e);
std::error_code make_error_code(error e);
const std::error_category& moca_category();

} // namespace moca

#endif // MOCA_ERROR_HPP
