//! \file num_get_3.cpp

// Copyright Vicente J. Botet Escriba 2014.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_RESULT_OF_USE_DECLTYPE
#include <boost/expected/expected_monad.hpp>
#include <iostream>
#include <streambuf>
#include <locale>
#include <sstream>
#include <type_traits>

#define expect(V, EXPR) \
auto BOOST_JOIN(expected,V) = EXPR; \
if (! BOOST_JOIN(expected,V).valid()) return BOOST_JOIN(expected,V).get_unexpected(); \
auto V = BOOST_JOIN(expected,V).value()

#define expect_void(V, EXPR) \
auto V = EXPR; \
if (! V.valid()) return V.get_unexpected(); \

template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
struct ios_range {
  InputIterator begin;
  InputIterator end;
  std::ios_base& ios;
//  ios_range(InputIterator f, InputIterator e, std::ios_base& ios)
//  : begin(f), end(e), ios(ios) {}
  ios_range(std::basic_stringstream<CharT>& is)
  : begin(is), end(), ios(is) {}
};


/**
 *
 * @param r the range
 * @Returns the expect Num value if the parse succeeds or the iostate is the parse fails.
 */

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<Num, std::ios_base::iostate> get_num(ios_range<CharT, InputIterator>& r) {

  std::ios_base::iostate err = std::ios_base::goodbit;
  Num v;
  r.begin = std::use_facet<std::num_get<CharT, InputIterator> >(r.ios.getloc()).get(r.begin, r.end, r.ios, err, v);
  if (err & (std::ios_base::badbit | std::ios_base::failbit)) {
    return boost::make_unexpected(err);
  }
  return v;
}

// todo finish this function
template <class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<void, std::ios_base::iostate>
matchedString(std::string str, ios_range<CharT, InputIterator>& r) {
  if (*r.begin != str[0]) {
      return boost::make_unexpected(std::ios_base::failbit);
  }
  ++r.begin;
  if (*r.begin != str[1]) {
      return boost::make_unexpected(std::ios_base::failbit);
  }
  ++r.begin;
  return boost::expected<void, std::ios_base::iostate>(boost::in_place2);
}

/**
 *
 * @param r the range
 * @Returns the expect pair<Num,Num> if the parse succeeds or the iostate is the parse fails.
 */

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval(ios_range<CharT, InputIterator>& r) {
  auto  f = get_num<Num>(r);
  if (! f.valid()) return f.get_unexpected();

  auto  m = matchedString("..", r);
  if (! m.valid()) return m.get_unexpected();

  auto  l = get_num<Num>(r);
  if (! l.valid()) return l.get_unexpected();

  return std::make_pair(f.value(), l.value());
}

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval2(ios_range<CharT, InputIterator>& r)
{
  expect(f, get_num<Num>(r));
  expect_void(m, matchedString("..", r));
  expect(l, get_num<Num>(r));
  return std::make_pair(f, l);
}

#if 0
template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval2(ios_range<CharT, InputIterator>& r)
{
  auto f = expect get_num<Num>(r);
           expect matchedString("..", r);
  auto l = expect get_num<Num>(r);
  return std::make_pair(f, l);
}
#endif


template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval3(ios_range<CharT, InputIterator>& r)
{
  return get_num<Num>(r)
   .bind( [&r](Num f)
    {
      return matchedString("..", r).bind([f]() { return f; });
    }
  ).bind( [&r](Num f)
    {
      return get_num<Num>(r).bind([f](Num l) { return std::make_pair(f,l); });
    }
  );
}

#if 0

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval3(ios_range<CharT, InputIterator>& r)
{
  return get_num<Num>(r)
    & [&r](Num f) { return matchedString("..", r) & [f]() { return f; }; }
    & [&r](Num f) { return get_num<Num>(r) & [f](Num l) { return std::make_pair(f,l); }; }
    ;
}

#endif

template <class T>
struct identity_t {
  T value;
  identity_t(T v) : value(v) {}
  T operator()() { return value; }
};

template <class T>
identity_t<T> identity(T v) { return identity_t<T>(v); }


template <class T>
struct lpair_t {
  T l;
  lpair_t(T v) : l(v) {}
  template <class U>
  std::pair<T,U> operator()(U r) { return std::make_pair(l, r); }
};

template <class T>
lpair_t<T> lpair(T v) { return lpair_t<T>(v); }

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval4(ios_range<CharT, InputIterator>& r)
{
  return get_num<Num>(r)
     .catch_error([](std::ios_base::iostate st) {
          std::cout << __FILE__ << "[" << __LINE__ << "] " << st << std::endl;
          return boost::make_unexpected(st);
        }
    ).bind( [&r](Num f) { return matchedString("..", r).bind( identity(f) ); }
    ).bind( [&r](Num f) { return get_num<Num>(r).bind( lpair(f) ); }
    );
}

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval5(ios_range<CharT, InputIterator>& r)
{
  using namespace boost::functional::monad_error;

  return ( get_num<Num>(r)
    | [](std::ios_base::iostate st) {
          std::cout << __FILE__ << "[" << __LINE__ << "] " << st << std::endl;
          return boost::make_unexpected(st);
        }
    )
    & [&r](Num f) { return matchedString("..", r).bind( identity(f) ); }
    & [&r](Num f) { return get_num<Num>(r).bind( lpair(f) ); }
    ;
}

#if 0

template <class Num, class CharT=char, class InputIterator = std::istreambuf_iterator<CharT> >
boost::expected<std::pair<Num,Num>, std::ios_base::iostate> get_interval4(ios_range<CharT, InputIterator>& r)
{
  return get_num<Num>(r)
    & [&r](Num f) { return matchedString("..", r) & identity(f) ; }
    & [&r](Num f) { return get_num<Num>(r) & lpair(f); }
    ;
}

#endif

int main()
{
  {
    std::stringstream is("1");
    ios_range<> r(is);
    auto x = get_num<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 5;
    }
    std::cout << *x << std::endl;
  }
  {
    std::stringstream is("1..3");
    ios_range<> r(is);
    auto x = get_interval<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 1;
    }
    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }

  {
    std::stringstream is("1..3");
    ios_range<> r(is);
    auto x = get_interval2<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 2;
    }

    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }

  {
    std::stringstream is("1..3");
    ios_range<> r(is);
    auto x = get_interval3<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 3;
    }

    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }
  {
      std::stringstream is("1..3 4..5");
      ios_range<> r(is);
      using namespace boost::functional::monad;

      auto x = get_interval3<long>(r);
      if (!x.valid()) {
        std::cout << x.error() << std::endl;
        return 7;
      }
      matchedString(" ", r);
      auto y = mdo(x, get_interval3<long>(r));
      if (!y.valid()) {
        std::cout << y.error() << std::endl;
        return 8;
      }

      std::cout << y.value().first << ".." << y.value().second << std::endl;
  }
  {
    std::stringstream is("1..3 4..5");
    ios_range<> r(is);
    using namespace boost::functional::monad;

    auto x = get_interval3<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 7;
    }
    matchedString(" ", r);
    auto y = x >> get_interval3<long>(r);
    if (!y.valid()) {
      std::cout << y.error() << std::endl;
      return 8;
    }

    std::cout << y.value().first << ".." << y.value().second << std::endl;
  }
  {
    std::stringstream is("1..3");
    ios_range<> r(is);
    auto x = get_interval4<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 4;
    }

    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }
  {
    std::stringstream is("1..3");
    ios_range<> r(is);
    auto x = get_interval5<long>(r);
    if (!x.valid()) {
      std::cout << x.error() << std::endl;
      return 4;
    }

    std::cout << x.value().first << ".." << x.value().second << std::endl;
  }

  return 0;
}

