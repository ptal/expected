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
#include <type_traits>

namespace boost {
// to be moved to monads.hpp

template <class T>
struct monad_traits;

template <class T>
struct monad;

template <class T>
struct is_monad : std::false_type {
};
template <class T>
struct is_monad<monad<T> > : std::true_type {
};

template <class T>
monad<typename std::decay<T>::type> make_monad(T&& v) {
  return monad<typename std::decay<T>::type>(std::forward<T>(v));
}

template <class T>
struct monad {
  typedef T value_type;
  T v;
  monad(T&& v) : v(std::forward<T>(v)) {}

  operator T() const { return v; }

  template <class F>
  auto then(F&& f) const
  -> decltype(make_monad(monad_traits<value_type>::then(v, std::forward<F>(f))))  {
    return    make_monad(monad_traits<value_type>::then(v, std::forward<F>(f)));
  }

};

template <class T, class F>
auto operator|(monad<T> m, F&& f)
  -> decltype(m.then(std::forward<F>(f)))  {
    return  m.then(std::forward<F>(f));
}


// to be moved to expected.hpp
template <class T, class E>
struct monad_traits<expected<T, E> >
{
  typedef expected<T, E> monad_type;
  typedef T value_type;
  typedef E error_type;

  static bool valid(monad_type m) {
    return m.valid();
  }
  static value_type get(monad_type m) {
    return m.value();
  }
  static error_type error(monad_type m) {
    return m.error();
  }
  static exceptional<error_type> get_exceptional(monad_type m) {
    return m.get_exceptional();
  }

  template <class F>
  struct result_type {
    typedef expected<typename std::result_of<F(value_type)>::type, E> type;
  };

  template <class F>
  static typename result_type<F>::type then(monad_type m, F f) {
    return m.then(f);
  }
};

} // boost

////////////

template <class I, typename T, class E >
using pair_expected = std::pair<I, boost::expected<T,E> > ;

template <class E, class I, typename T  >
pair_expected<I,T,E> make_pair_expected(I i, T v) {
  return std::make_pair(i, boost::expected<T,E>(v));
}

template <class T, class I, typename E  >
pair_expected<I,T,E> make_pair_expected_from_error(I i, E e) {
  return std::make_pair(i, boost::expected<T,E>(boost::make_error(e)));
}


// making pair_expected a monad

namespace boost {

template <class T, class I, typename E  >
struct monad_traits<pair_expected<I,T,E> >
{
  typedef pair_expected<I,T,E> monad_type;
  typedef std::pair<I,T> value_type;
  typedef E error_type;

  static bool valid(monad_type m) {
    return m.second.valid();
  }

  static value_type get(monad_type m) {
    return std::make_pair(m.first, m.second.value());
  }

  static error_type error(monad_type m) {
    return m.second.error();
  }
  static exceptional<error_type> get_exceptional(monad_type m) {
    return m.second.get_exceptional();
  }

  template <class F>
  struct result_type {
    typedef typename std::result_of<F(value_type)>::type type;
  };

  template <class F>
  static typename result_type<F>::type
  then(monad_type m, F f) {
    typedef typename std::result_of<F(value_type)>::type result_type;
    typedef typename result_type::second_type expected_type;
    if (valid(m)) {
      return f(get(m));
    }
    return make_pair(m.first, expected_type(get_exceptional(m)));
  }

};

} // boost

//template <class T, class I, typename E, class F>
//auto operator|(pair_expected<T, I, E> m, F&& f)
//  -> decltype(boost::monad_traits<pair_expected<T, I, E> >::then(m, std::forward<F>(f)))  {
//    return    boost::monad_traits<pair_expected<T, I, E> >::then(m, std::forward<F>(f));
//}

namespace boost {

template <class T>
struct monad_wrapper;

template <class T, class I, typename E  >
struct monad_wrapper<pair_expected<I,T,E> >
{
  typedef pair_expected<I,T,E> monad_type;
  typedef std::pair<I,T> value_type;
  typedef E error_type;

  monad_type m;
  monad_wrapper(monad_type v) : m(v) {}

  operator monad_type() const { return m; }

  bool valid() const {
    return m.second.valid();
  }

  value_type value() const {
    return std::make_pair(m.first, m.second.value());
  }

  error_type error() const {
    return m.second.error();
  }
  exceptional<error_type> get_exceptional() const {
    return m.second.get_exceptional();
  }

  template <class F>
  struct result_type {
    typedef typename std::result_of<F(value_type)>::type type;
  };

  template <class F>
  typename result_type<F>::type
  then(F f) const {
    typedef typename std::result_of<F(value_type)>::type result_type;
    typedef typename result_type::second_type expected_type;
    if (valid()) {
      return f(value());
    }
    return make_pair(m.first, expected_type(get_exceptional()));
  }

//  template <class F>
//  friend
//  auto operator|(pair_expected<T, I, E> m, F&& f)
//    -> decltype(boost::monad_wrapper<pair_expected<T, I, E> >(m).then(std::forward<F>(f)))  {
//      return    boost::monad_wrapper<pair_expected<T, I, E> >(m).then(std::forward<F>(f));
//  }

};

} // boost

template <class T, class I, typename E, class F>
auto operator|(pair_expected<T, I, E> m, F&& f)
  -> decltype(boost::monad_wrapper<pair_expected<T, I, E> >(m).then(std::forward<F>(f)))  {
    return    boost::monad_wrapper<pair_expected<T, I, E> >(m).then(std::forward<F>(f));
}


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
  pair_expected<iter_type, Num, std::ios_base::iostate> get(InputIterator s,
      InputIterator e, std::ios_base& ios) const
  {
    using namespace boost;
    using namespace std;

    ios_base::iostate err = std::ios_base::goodbit;
    Num v;
    //auto is = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc());

    InputIterator it = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc()).get(s, e, ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit)) {
      return make_pair_expected_from_error<Num>(it, err);
    }
    return make_pair_expected<std::ios_base::iostate>(it, v);
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
pair_expected<InputIterator, long, std::ios_base::iostate>
matchedString(std::string str, InputIterator s, InputIterator e) {
  if (*s != str[0]) {
      return make_pair_expected_from_error<long>(s, std::ios_base::goodbit);
  }
  ++s;
  if (*s != str[1]) {
      return make_pair_expected_from_error<long>(s, std::ios_base::goodbit);
  }
  ++s;
  return  make_pair_expected<std::ios_base::iostate>(s, 0L); // todo check the match
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
  pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get(InputIterator s,
      InputIterator e, std::ios_base& ios)  const
  {
    using namespace boost;
    typedef std::pair<Num, Num> value_type;
    //typedef typename std::result_of<F(std::pair<I, T>)>::type result_type;
    //typedef typename result_type::second_type expected_type;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto  f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);
    if (! f.second) return make_pair_expected_from_error<value_type>(f.first, f.second.error());

    auto  m = matchedString("..", f.first, e);
    if (! m.second) return make_pair_expected_from_error<value_type>(m.first, m.second.error());
    //auto l = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
    auto l = ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);
    if (! l.second) return make_pair_expected_from_error<value_type>(l.first, l.second.error());

    value_type tmp = std::make_pair(*f.second, *l.second);
    return make_pair_expected<std::ios_base::iostate>(l.first, tmp);
  }

  template <typename Num>
  pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate>
  get2(InputIterator s, InputIterator e, std::ios_base& ios)  const
  {
    using namespace boost;
    typedef std::pair<Num, Num> value_type;

    typedef monad_traits<pair_expected<iter_type, Num, std::ios_base::iostate> > t1;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto f = ::NumGet<char_type, iter_type>().template get<Num> (s, e, ios);
    auto m = t1::then(f, [e](std::pair<iter_type,Num> f) {
      return matchedString("..", f.first, e);
    });
    auto l = t1::then(m, [&ios, e](std::pair<iter_type, Num> m) {
      //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
      return ::NumGet<char_type, iter_type>().template get<Num> (m.first, e, ios);
    });
    return t1::then(l, [f](std::pair<iter_type,Num> l) {
      value_type tmp(*f.second, l.second);
      return make_pair_expected<std::ios_base::iostate>(l.first, tmp);
    });

  }

  template <typename Num>
  pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get3(InputIterator s,
      InputIterator e, std::ios_base& ios)  const
  {
    using namespace boost;
    typedef std::pair<Num, Num> value_type;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto  f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);

    return make_monad(std::move(f))
      | [e](std::pair<iter_type,Num> f) {

        return matchedString("..", f.first, e);

      } | [&ios, e](std::pair<iter_type,Num> m) {

        //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
        return ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);

      } | [f](std::pair<iter_type,Num> l) {

        value_type tmp(*f.second, l.second);
        return make_pair_expected<std::ios_base::iostate>(l.first, tmp);

      };
  }

  template <typename Num>
  pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get4(InputIterator s,
      InputIterator e, std::ios_base& ios)  const
  {
    using namespace boost;
    typedef std::pair<Num, Num> value_type;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto  f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);

    return make_monad(std::move(f))
      .then([e](std::pair<iter_type,Num> f) {

        return matchedString("..", f.first, e);

      } ).then( [&ios, e](std::pair<iter_type,Num> m) {

        //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
        return ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);

      } ).then( [f](std::pair<iter_type,Num> l) {

        value_type tmp(*f.second, l.second);
        return make_pair_expected<std::ios_base::iostate>(l.first, tmp);

      });
  }
  template <typename Num>
  pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get5(InputIterator s,
      InputIterator e, std::ios_base& ios)  const
  {
    using namespace boost;
    typedef std::pair<Num, Num> value_type;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto  f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);

    return f
      | [e](std::pair<iter_type,Num> f) {

        return matchedString("..", f.first, e);

      } | [&ios, e](std::pair<iter_type,Num> m) {

        //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
        return ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);

      } | [f](std::pair<iter_type,Num> l) {

        value_type tmp(*f.second, l.second);
        return make_pair_expected<std::ios_base::iostate>(l.first, tmp);

      };
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
    NumGet<>::iter_type end;
    auto x = facet.get<long> (is, end, is);
    if (!x.second) {
      std::cout << int(x.second.error()) << std::endl;
      return 1;
    }

    std::cout << *x.second << std::endl;
  }
  {

    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get<long> (is, end, is);
    auto x = ::NumIntervalGet<>().get<long> (is, end, is);
    if (!x.second) {
      std::cout << x.second.error() << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }
  {

    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get2<long> (is, end, is);
    auto x = ::NumIntervalGet<>().get2<long> (is, end, is);
    if (!x.second) {
      std::cout << x.second.error() << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }
  {

    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get3<long> (is, end, is);
    auto x = ::NumIntervalGet<>().get3<long> (is, end, is);
    if (!x.second) {
      std::cout << x.second.error() << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }
  {

    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get4<long> (is, end, is);
    auto x = ::NumIntervalGet<>().get4<long> (is, end, is);
    if (!x.second) {
      std::cout << x.second.error() << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }
  {

    std::stringstream is("1..3");
    NumIntervalGet<>::iter_type end;
    //auto x = std::use_facet< ::NumIntervalGet<> >(is.getloc()).get5<long> (is, end, is);
    auto x = ::NumIntervalGet<>().get5<long> (is, end, is);
    if (!x.second) {
      std::cout << x.second.error() << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }

  return 0;
}

