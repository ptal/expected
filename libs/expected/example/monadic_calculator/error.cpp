// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Pierre Talbot

#include "error.hpp"

namespace moca{

const std::array<std::string, num_error> error_category::error_messages = {{
  "Value must hold in a 32 bits integer", // integer_overflow
  "Unknown symbol", // unknown_symbol
  "Overflow computation in the 32 bits integer domain", // operation_overflow
  "Division by zero", // division_by_zero
  "Operand was expected", // expected_operand
  "Operator was expected", // expected_operator
}};

const char* error_category::name() const noexcept
{
  return "moca";
}

std::string error_category::message(int ev) const
{
  if(ev < 0 || ev >= static_cast<int>(error_messages.size()))
    return std::string("Unknown error");
  return error_messages[ev];
}

// MOCA Error condition factory.
std::error_condition make_error_condition(error e)
{
  return std::error_condition(e, moca_category());
}

// MOCA Error code factory.
std::error_code make_error_code(error e)
{
  return std::error_code(e, moca_category());
}

const std::error_category& moca_category()
{
  static const moca::error_category moca_category_const;
  return moca_category_const;
}

} // namespace moca
