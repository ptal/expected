//! \file getInt.cpp

// Copyright Vicente J. Botet Escriba 2013.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/expected/expected.hpp>
#include <iostream>
#include <streambuf>
#include <locale>
#include <sstream>

/**
 * num_get facet using the expected interface
 */
template <class CharT, class InputIterator = std::istreambuf_iterator<CharT> >
struct num_get: public std::locale::facet
{
public:
  /**
   * Type of character the facet is instantiated on.
   */
  typedef CharT char_type;
  /**
   * Type of character string passed to member functions.
   */
  typedef std::basic_string<CharT> string_type;
  /**
   * Type of iterator used to scan the character buffer.
   */
  typedef InputIterator iter_type;

  explicit num_get(size_t refs = 0) :
    std::locale::facet(refs)
  {
  }

  /**
   *
   * @param s start input stream iterator
   * @param end end input stream iterator
   * @param ios a reference to a ios_base
   * @Returns a 'complex' type consisting of
   *  * the iterator pointing just beyond the last character that can be determined to be part of a valid name
   *  * the value obtained or the error code
   */

  template <typename Num>
  boost::expected<std::pair<iter_type, Num>, std::pair<iter_type, std::ios_base::iostate>> get(InputIterator s,
      InputIterator e, std::ios_base& ios)
  {
    using namespace boost;
    using namespace std;
    typedef boost::expected<std::pair<iter_type, Num>, std::pair<iter_type, std::ios_base::iostate>> result_type;

    ios_base::iostate err = std::ios_base::goodbit;
    Num v;
    InputIterator it = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc()).get(s, e, ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit))
    {
      return result_type(boost::exceptional, make_pair(it, err));
    }
    else
      return result_type(make_pair(it, v));
  }

  /**
   * Unique identifier for this type of facet.
   */
  static std::locale::id id;
};

template <class CharT, class InputIterator>
std::locale::id num_get<CharT, InputIterator>::id;

using namespace boost;

int main()
{

  std::stringstream is("123");
  num_get<char> Getter;
  num_get<char>::iter_type end;
  auto x = Getter.get<long> (is, end, is);
  if (!x)
  {
    std::cout << int(x.get_error().second) << std::endl;
    return 1;
  }

  std::cout << x.get().second << std::endl; // just "re"throw the stored exception

  return 0;
}

