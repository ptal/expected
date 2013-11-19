//! \file num_get_2.cpp

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
    typedef std::pair<iter_type, boost::expected<Num, std::ios_base::iostate> > type;
    static type make(iter_type it, Num n) {
      return std::make_pair(it, boost::expected<Num, std::ios_base::iostate>(n));
    }
    static type make(iter_type it, std::ios_base::iostate errc) {
      return std::make_pair(it, boost::expected<Num, std::ios_base::iostate>(boost::exceptional, errc));
    }
    static bool valid(type v) {
      return v.second;
    }
    static iter_type iterator(type v) {
      return v.first;
    }
    static Num value(type v) {
      return *v.second;
    }
    static std::ios_base::iostate error(type v) {
      return *v.second.get_error();
    }


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
    typedef get_result_type<Num> fact_type;
    //typedef typename fact_type::type result_type;

    ios_base::iostate err = std::ios_base::goodbit;
    Num v;
    //auto is = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc());

    InputIterator it = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc()).get(s, e, ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit))
      return fact_type::make(it, err);
    return fact_type::make(it, v);
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
std::pair<InputIterator, boost::expected<int, std::ios_base::iostate> >
matchedString(std::string, InputIterator s, InputIterator e) {
  return  std::make_pair(s, boost::expected<int, std::ios_base::iostate>(0)); // todo check the match
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
    typedef std::pair<iter_type, boost::expected<std::pair<Num, Num>, std::ios_base::iostate> > type;
    static type make(iter_type it, Num f, Num l) {
      return std::make_pair(it, boost::expected<std::pair<Num, Num>, std::ios_base::iostate> (std::make_pair(f, l)));
    }
    static type make(iter_type it, std::ios_base::iostate errc) {
      return std::make_pair(it, boost::expected<std::pair<Num, Num>, std::ios_base::iostate>(boost::exceptional, errc));
    }
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
    typedef get_result_type<Num> fact_type;

    //auto facet = std::use_facet< ::num_get<char_type, iter_type> >(ios.getloc());
    auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    if (! f.second) return fact_type::make(f.first, f.second.get_error());
    auto  m = matchedString("..", f.first, e);
    if (! m.second) return fact_type::make(m.first, m.second.get_error());
    auto l = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
    if (! l.second) return fact_type::make(l.first, l.second.get_error());

    return fact_type::make(l.first, *f.second, *l.second);
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
    if (!x.second) {
      std::cout << int(x.second.get_error()) << std::endl;
      return 1;
    }

    std::cout << *x.second << std::endl;
  }
  if (0)
  {
    std::stringstream is("1..3");
    // auto facet = std::use_facet< ::NumIntervalGet<> >(is.getloc());
    ::NumIntervalGet<> facet;
    NumIntervalGet<>::iter_type end;
    NumIntervalGet<>::get_result_type<long>::type x = facet.get<long> (is, end, is);
    if (!x.second) {
      std::cout << int(x.second.get_error()) << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }

  return 0;
}

