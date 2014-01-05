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
template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
struct NumGet: public std::locale::facet
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

  explicit NumGet(size_t refs = 0) :
    std::locale::facet(refs)
  {
  }

  template <typename Num>
  struct get_result_type {
    typedef boost::expected<std::pair<iter_type, Num>, std::pair<iter_type, std::ios_base::iostate> > type;
  };
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
  typename get_result_type<Num>::type get(InputIterator s,
      InputIterator e, std::ios_base& ios) const
  {
    using namespace boost;
    using namespace std;
    typedef typename get_result_type<Num>::type result_type;
    typedef std::pair<iter_type, std::ios_base::iostate> exceptional_type;

    ios_base::iostate err = std::ios_base::goodbit;
    Num v;
    //auto is = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc());

    InputIterator it = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc()).get(s, e, ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit))
      return boost::make_error(make_pair(it, err));
    return result_type(make_pair(it, v));
  }

  /**
   * Unique identifier for this type of facet.
   */
  static std::locale::id id;
};

template <class CharT, class InputIterator>
std::locale::id NumGet<CharT, InputIterator>::id;


// todo finish this function
template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<InputIterator, // todo solve the issues when using InputIterator instead of std::pair<InputIterator, int>
                std::pair<InputIterator, std::ios_base::iostate>>
matchedString(std::string, InputIterator s, InputIterator e) {
  return boost::expected<InputIterator,
               std::pair<InputIterator, std::ios_base::iostate>>(s); // todo check the match
}

/**
 * num_get facet using the expected interface
 */
template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
struct NumIntervalGet: public std::locale::facet
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

  explicit NumIntervalGet(size_t refs = 0) :
    std::locale::facet(refs)
  {
  }

  template <typename Num>
  struct get_result_type {
    typedef boost::expected<std::pair<iter_type, std::pair<Num, Num>>,
                            std::pair<iter_type, std::ios_base::iostate>
            > type;
  };

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
  typename get_result_type<Num>::type get(InputIterator s,
      InputIterator e, std::ios_base& ios)  const
  {
    using namespace boost;
    using namespace std;
    typedef typename get_result_type<Num>::type result_type;

    auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    if (! f) return f.get_exceptional();
    auto  m = matchedString("..", f->first, e);
    if (! m) return m.get_exceptional();
    auto l = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(*m, e, ios);
    if (! l) return l.get_exceptional();

    return result_type(make_pair(l->first, make_pair(f->second, l->second)));
  }

  /**
   * Unique identifier for this type of facet.
   */
  static std::locale::id id;
};

template <class CharT, class InputIterator>
std::locale::id NumIntervalGet<CharT, InputIterator>::id;


using namespace boost;

int main()
{
  {
    std::stringstream is("123");
    ::NumGet<> facet;
    NumGet<char>::iter_type end;
    NumGet<char>::get_result_type<long>::type x = facet.get<long> (is, end, is);
    if (!x) {
      std::cout << int(x.error().second) << std::endl;
      return 1;
    }

    std::cout << x->second << std::endl;
  }
  if (0)
  {
    std::stringstream is("1..3");
    // auto facet = std::use_facet< ::NumIntervalGet<> >(is.getloc());
    ::NumIntervalGet<> facet;
    NumIntervalGet<>::iter_type end;
    NumIntervalGet<>::get_result_type<long>::type x = facet.get<long> (is, end, is);
    if (!x) {
      std::cout << int(x.error().second) << std::endl;
      return 1;
    }

    std::cout << x->second.first << ".." << x->second.second << std::endl;
  }

  return 0;
}

