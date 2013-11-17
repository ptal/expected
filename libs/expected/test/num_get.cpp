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
struct my_num_get: public std::locale::facet
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

  explicit my_num_get(size_t refs = 0) :
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

    ios_base::iostate err = std::ios_base::goodbit;
    Num v;
    //auto is = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc());

    InputIterator it = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc()).get(s, e, ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit))
      return result_type(boost::exceptional, make_pair(it, err));
    return result_type(make_pair(it, v));
  }

  /**
   * Unique identifier for this type of facet.
   */
  static std::locale::id id;
};

template <class CharT, class InputIterator>
std::locale::id my_num_get<CharT, InputIterator>::id;

template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<InputIterator, int>, // todo solve the issues when using InputIterator instead of std::pair<InputIterator, int>
                std::pair<InputIterator, std::ios_base::iostate>>
matchedString(std::string, InputIterator s, InputIterator e) {
  return boost::expected<std::pair<InputIterator, int>,
               std::pair<InputIterator, std::ios_base::iostate>>(std::make_pair(s, 0)); // todo check the match
}

/**
 * num_get facet using the expected interface
 */
template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
struct num_interval_get: public std::locale::facet
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

  explicit num_interval_get(size_t refs = 0) :
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

    //auto facet = std::use_facet<::num_get<char_type, iter_type> >(ios.getloc());
    auto  f = std::use_facet<::my_num_get<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    if (! f) return result_type(boost::exceptional, f.get_error());
    //auto  m = std::use_facet<::my_num_get<char_type, iter_type> >(ios.getloc()).template get<Num>(f->first, e, ios);
    auto  m = matchedString("..", f->first, e); // todo finish this function
    if (! m) return result_type(boost::exceptional, m.get_error());
    auto l = std::use_facet<::my_num_get<char_type, iter_type> >(ios.getloc()).template get<Num>(m->first, e, ios);
    if (! l) return result_type(boost::exceptional, l.get_error());

    return result_type(make_pair(l->first, make_pair(f->second, l->second)));
  }

  /**
   * Unique identifier for this type of facet.
   */
  static std::locale::id id;
};

template <class CharT, class InputIterator>
std::locale::id num_interval_get<CharT, InputIterator>::id;


using namespace boost;

int main()
{
  {
    std::stringstream is("123");
    ::my_num_get<> facet;
    my_num_get<char>::iter_type end;
    my_num_get<char>::get_result_type<long>::type x = facet.get<long> (is, end, is);
    if (!x) {
      std::cout << int(x.get_error().second) << std::endl;
      return 1;
    }

    std::cout << x->second << std::endl;
  }
  if (0)
  {
    std::stringstream is("1 2 3");
    // auto facet = std::use_facet<::num_interval_get<> >(is.getloc());
    ::num_interval_get<> facet;
    num_interval_get<>::iter_type end;
    num_interval_get<>::get_result_type<long>::type x = facet.get<long> (is, end, is);
    if (!x) {
      std::cout << int(x.get_error().second) << std::endl;
      return 1;
    }

    std::cout << x->second.first << ".." << x->second.second << std::endl;
  }

  return 0;
}

