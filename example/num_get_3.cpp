//! \file num_get_3.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/expected/expected.hpp>
#include <iostream>
#include <streambuf>
#include <locale>
#include <sstream>
#include <type_traits>

#define expect(V, EXPR) \
auto BOOST_JOIN(expected,V) = EXPR; \
if (! BOOST_JOIN(expected,V).valid()) return BOOST_JOIN(expected,V).get_unexpected(); \
auto V =BOOST_JOIN(expected,V).value()


#define expect_void(V, EXPR) \
auto V = EXPR; \
if (! V.valid()) return V.get_unexpected(); \

template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
struct ios_range {
  InputIterator begin;
  InputIterator end;
  std::ios_base& ios;
  ios_range(InputIterator f, InputIterator e, std::ios_base& ios)
  : begin(f), end(e), ios(ios) {}
};


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

  /**
   * Type of the expected value.
   */
  template <class Num>
  using expected_type = boost::expected<Num, std::ios_base::iostate>;

  /**
   * Type of the range state.
   */
  typedef ios_range<CharT, InputIterator> range_type;

  explicit NumGet(size_t refs = 0) :
    std::locale::facet(refs)
  {
  }

  /**
   *
   * @param r the range
   * @Returns the expect Num value if the parse succeeds or the iostate is the parse fails.
   */

  template <typename Num>
  expected_type<Num> get(range_type& r) const
  {

    std::ios_base::iostate err = std::ios_base::goodbit;
    Num v;

    r.begin = std::use_facet<std::num_get<char_type, iter_type> >(r.ios.getloc()).get(r.begin, r.end, r.ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit)) {
      return boost::make_unexpected(err);
    }
    //return expected_type<Num>(v);
    return v;
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
boost::expected<void, std::ios_base::iostate>
matchedString(std::string str, ios_range<CharT, InputIterator>& r) {
  if (*r.begin != str[0]) {
      return boost::make_unexpected(std::ios_base::goodbit);
  }
  ++r.begin;
  if (*r.begin != str[1]) {
      return boost::make_unexpected(std::ios_base::goodbit);
  }
  ++r.begin;
  return boost::make_expected<std::ios_base::iostate>();
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

  /**
   * Type of the expected value.
   */
  template <class Num>
  using expected_type = boost::expected<std::pair<Num,Num>, std::ios_base::iostate>;

  /**
   * Type of the range state.
   */
  typedef ios_range<CharT, InputIterator> range_type;


  explicit NumIntervalGet(size_t refs = 0) :
    std::locale::facet(refs)
  {
  }

  /**
   *
   * @param r the range
   * @Returns the expect pair<Num,Num> if the parse succeeds or the iostate is the parse fails.
   */

  template <typename Num>
  expected_type<Num> get(range_type& r) const
  {
    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(r);
    auto  f = ::NumGet<char_type, iter_type>().template get<Num>(r);
    if (! f.valid()) return f.get_unexpected();

    auto  m = matchedString("..", r);
    if (! m.valid()) return m.get_unexpected();

    //auto l = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(r);
    auto l = ::NumGet<char_type, iter_type>().template get<Num>(r);
    if (! l.valid()) return l.get_unexpected();

    //return expected_type<Num>(std::make_pair(f.value(), l.value()));
    return std::make_pair(f.value(), l.value());
  }

  template <typename Num>
  expected_type<Num> get2(range_type& r) const
  {
    typedef ::NumGet<char_type, iter_type> num_get;
    expect(f, num_get().template get<Num>(r));
    expect_void(m, matchedString("..", r));
    expect(l, num_get().template get<Num>(r));
    return std::make_pair(f, l);
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
    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    NumIntervalGet<>::range_type r(is, end, is);
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get<long> (r);
    auto x = ::NumIntervalGet<>().get<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 1;
    }

    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }

  {
    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    NumIntervalGet<>::range_type r(is, end, is);
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get<long> (r);
    auto x = ::NumIntervalGet<>().get2<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 2;
    }

    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }

  return 0;
}

