// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Pierre Talbot

#include "error.hpp"

#include <boost/expected/expected.hpp>
#include <boost/functional/do_yield.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cctype>
#include <functional>
#include <algorithm>
#include <system_error>

// Output in out a std::error_condition.
template <class Output>
Output& operator<<(Output& out, const std::error_condition& e)
{
  return (out << e.message());
}

/* Print an expected, the value and error of this expected
must implement the << operator. */
template <class Output, class T, class E>
Output& operator<<(Output& out, const boost::expected<T, E>& v)
{
  if(v.valid())
    return out << *v;
  else
    return out << v.error();
}

// moca stands for monadic calculator.
namespace moca{

// Expected helpers for the std::error_condition error type.
template <class T>
using expected = boost::expected<T, std::error_condition>;

boost::unexpected_type<std::error_condition> make_unexpected_condition(error e)
{
  return boost::make_unexpected(make_error_condition(e));
}

// Phase 1: Lexical analysis.

// Tokens of an arithmetic expression.
enum class operator_token {MUL, DIV};
typedef std::int32_t int32_token;

typedef boost::variant<
  operator_token,
  int32_token> token_t;

typedef std::vector<token_t> tokens_t;

/** Safely convert a string to a integer. The range [b, e[ must only contains digits.
Error:
 * integer_overflow: If the value cannot be represented in a 32 bits integer.
*/
template <class Iterator>
expected<token_t> safe_string_to_int32(Iterator b, Iterator e)
{
  try
  {
    return token_t(boost::lexical_cast<std::int32_t>(boost::iterator_range<Iterator>(b, e)));
  } 
  catch (const boost::bad_lexical_cast&)
  {
    return make_unexpected_condition(integer_overflow);
  }
}

template <class Iterator>
expected<token_t> read_integer(Iterator& b, Iterator e)
{
  Iterator start = b;
  for(; b != e && std::isdigit(*b); ++b)
  {}

  return safe_string_to_int32(start, b);
}

/* Read an arithmetic operator at *b and increment the iterator.
Error:
  * unknown_symbol: If the operator is unknown.
*/
template <class Iterator>
expected<token_t> read_operator(Iterator& b, Iterator e)
{
  char symbol = *b;
  ++b;
  switch (symbol)
  {
    case '/': return token_t(operator_token::DIV);
    case '*': return token_t(operator_token::MUL);
    default:
      return make_unexpected_condition(unknown_symbol);
  }
}

template <class Iterator>
expected<token_t> next_token(Iterator& b, Iterator e)
{
  if(std::isdigit(*b))
  {
    return read_integer(b, e);
  }
  else
  {
    return read_operator(b, e);
  }
}

template <class Iterator>
Iterator eat_spaces(Iterator b, Iterator e)
{
  return std::find_if_not(b, e, [](char x){ return std::isspace(x); });
}

tokens_t push_token(const token_t& x, tokens_t& tokens)
{
  tokens.push_back(x);
  return std::move(tokens);
}

template <class Iterator>
expected<tokens_t> tokenize(Iterator b, Iterator e)
{
  expected<tokens_t> etokens = tokens_t();
  for(b = eat_spaces(b, e); 
      b != e && etokens.valid();
      b = eat_spaces(b,e))
  {
    etokens = DO(
      token_t token, next_token(b, e),
      tokens_t tokens, etokens,
    YIELD(push_token(token, tokens)));
  }
  return etokens;
}

// Phase 2: Evaluation and parsing.

/* Multiplies u per v.

Error:
  * operation_overflow: If the result of u*v cannot be represented on a 32 bits integer.
*/
expected<std::int32_t> safe_multiplies(std::int32_t u, std::int32_t v)
{
  std::int32_t res = u * v;
  std::int64_t res2 = std::int64_t(u) * std::int64_t(v);
  if(res != res2)
    return make_unexpected_condition(operation_overflow);
  else
    return res;
}

/* Divide u per v.

Error:
  * division_by_zero: If v == 0.
*/
expected<std::int32_t> safe_divide(std::int32_t u, std::int32_t v)
{
  if(v == 0)
    return make_unexpected_condition(division_by_zero);
  return u/v;
}

/* Visit a token and return the integer value.

Error:
  * operand_expected: If the token is not an integer.
*/
class IntegerParser : public boost::static_visitor<expected<std::int32_t>>
{
public:
  expected<std::int32_t> operator()(std::int32_t value) const
  {
    return value;
  }

  expected<std::int32_t> operator()(operator_token) const
  {
    return make_unexpected_condition(expected_operand);
  }
};

/* Visit a token and return the operator.

Error:
  * operator_expected: If the token is not an operator.
*/
class OperatorParser : public boost::static_visitor<expected<operator_token>>
{
public:
  expected<operator_token> operator()(std::int32_t) const
  {
    return make_unexpected_condition(expected_operator);
  }

  expected<operator_token> operator()(operator_token token) const
  {
    return token;
  }
};

expected<std::int32_t> parse_integer(const token_t& x)
{
  return boost::apply_visitor(IntegerParser(), x);
}

expected<operator_token> parse_operator(const token_t& x)
{
  return boost::apply_visitor(OperatorParser(), x);
}

// Safely compute left op right.
expected<std::int32_t> evaluate(std::int32_t left, operator_token op, std::int32_t right)
{
  switch (op)
  {
    case operator_token::MUL:
      return safe_multiplies(left, right);
    default: // operator_token::DIV:
      return safe_divide(left, right);
  }
}

// Having the left value, parse the operator that must follows.
template <class Iterator>
expected<std::int32_t> parse_operator(std::int32_t left, Iterator b, Iterator e)
{
  if(b == e)
    return left;
  else
  {
    return
      DO(
        operator_token op, parse_operator(*b++),
        std::int32_t right, parse_expr(b, e),
      YIELD(evaluate(left, op, right)));
  }
}

// Parse and evaluate an arithmetic expression.
template <class Iterator>
expected<std::int32_t> parse_expr(Iterator b, Iterator e)
{
  if(b == e)
    return make_unexpected_condition(expected_operand);
  else
  {
    return
      DO(
        std::int32_t left, parse_integer(*b++),
      YIELD(parse_operator(left, b, e)));
  }
}

expected<std::int32_t> compute(const std::string& calculus)
{
  return
    DO(
      tokens_t tokens, tokenize(calculus.begin(), calculus.end()),
    YIELD(parse_expr(tokens.begin(), tokens.end())));
}

} // namespace moca

void prompt()
{
  std::cout << "> ";
}

int main()
{
  using namespace moca;
  std::string user_input;
  std::cout << "Use \"quit\" to exit.\n"
   "This calculator is only able to compute arithmetic expression with operator / and *.\n";
  prompt();
  while(getline(std::cin, user_input))
  {
    if(user_input == "quit")
      break;

    moca::expected<std::int32_t> i = compute(user_input);
    std::cout << i << std::endl;

    prompt();
  }
}
