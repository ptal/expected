//! \file num_get_2.cpp

// Copyright Vicente J. Botet Escriba 2013.

// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <boost/expected/expected_monad.hpp>
#include <iostream>
#include <streambuf>
#include <locale>
#include <sstream>
#include <type_traits>

template <class I, typename T, class E >
using pair_expected = std::pair<I, boost::expected<T,E> >;

template <class E, class I, typename T>
pair_expected<I, T, E> make_pair_expected(I i, T v)
{
  return std::make_pair(i, boost::expected<T, E>(v));
}

template <class T, class I, typename E>
pair_expected<I, T, E> make_pair_expected_from_error(I i, E e)
{
  return std::make_pair(i, boost::expected<T, E>(boost::make_unexpected(e)));
}

// making pair_expected a monad

namespace boost
{
  namespace monads
  {

    template <class T, class I, typename E, class U>
    struct bind<pair_expected<I, T, E>, U>
    {
      typedef pair_expected<I, U, E> type;
    };

    template <class T, class I, typename E>
    struct monad_traits<pair_expected<I, T, E> >
    {
    private:
      typedef pair_expected<I, T, E> monad_type;
      typedef std::pair<I, T> value_type;
      typedef E error_type;


      static BOOST_CONSTEXPR bool valid(monad_type m)
      {
        return m.second.valid();
      }

      static BOOST_CONSTEXPR value_type get(monad_type m)
      {
        return std::make_pair(m.first, m.second.value());
      }

      static BOOST_CONSTEXPR error_type error(monad_type m)
      {
        return m.second.error();
      }
      static BOOST_CONSTEXPR unexpected_type<error_type> get_unexpected(monad_type m)
      {
        return m.second.get_unexpected();
      }

      template <class F>
      struct result_type
      {
        typedef typename std::result_of<F(value_type)>::type type;
      };
    public:
      template <class F>
      static BOOST_CONSTEXPR typename result_type<F>::type mbind(monad_type m, F f)
      {
        typedef typename std::result_of<F(value_type)>::type result_type;
        typedef typename result_type::second_type expected_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
        if (valid(m))
        {
          return f(get(m));
        }
        return make_pair(m.first, expected_type(get_unexpected(m)));
#else
        typedef typename bind<monad_type, result_type>::type monad_result_type;
        return ( valid(m)
               ?  f(get(m))
               : make_pair(m.first, expected_type(get_unexpected(m)))
        );
#endif
      }
    };
  }
} // boost

template <class T, class I, typename E, class F>
auto operator&(pair_expected<T, I, E>& m, F&& f)
-> decltype(boost::monads::make_monad(std::forward<pair_expected<T, I, E>>(m)).mbind(std::forward<F>(f)))
{
  return boost::monads::make_monad(std::forward<pair_expected<T, I, E>>(m)).mbind(std::forward<F>(f));
}

/**
 * num_get facet using the expected interface
 */
template <class CharT = char, class InputIterator = std::istreambuf_iterator<CharT> >
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
  pair_expected<iter_type, Num, std::ios_base::iostate> get(InputIterator s, InputIterator e, std::ios_base& ios) const
  {
    using namespace boost;
    using namespace std;

    ios_base::iostate err = std::ios_base::goodbit;
    Num v;
    //auto is = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc());

    InputIterator it = std::use_facet<std::num_get<char_type, iter_type> >(ios.getloc()).get(s, e, ios, err, v);
    if (err & (std::ios_base::badbit | std::ios_base::failbit))
    {
      return make_pair_expected_from_error<Num> (it, err);
    }
    return make_pair_expected<std::ios_base::iostate> (it, v);
  }

  /**
   * Unique identifier for this type of facet.
   */
  static std::locale::id id;
};

template <class CharT, class InputIterator>
std::locale::id NumGet<CharT, InputIterator>::id;

// todo finish this function
template <class CharT = char, class InputIterator = std::istreambuf_iterator<CharT> >
pair_expected<InputIterator, long, std::ios_base::iostate> matchedString(std::string str, InputIterator s,
    InputIterator e)
{
  if (*s != str[0])
  {
    return make_pair_expected_from_error<long> (s, std::ios_base::goodbit);
  }
  ++s;
  if (*s != str[1])
  {
    return make_pair_expected_from_error<long> (s, std::ios_base::goodbit);
  }
  ++s;
  return make_pair_expected<std::ios_base::iostate> (s, 0L); // todo check the match
}

/**
 * num_get facet using the expected interface
 */
template <class CharT = char, class InputIterator = std::istreambuf_iterator<CharT> >
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
  pair_expected<iter_type, std::pair<Num, Num>, std::ios_base::iostate> get(InputIterator s, InputIterator e,
      std::ios_base& ios) const
  {
    using namespace boost;
    typedef std::pair<Num, Num> value_type;
    //typedef typename std::result_of<F(std::pair<I, T>)>::type result_type;
    //typedef typename result_type::second_type expected_type;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto f = ::NumGet<char_type, iter_type>().template get<Num> (s, e, ios);
    if (!f.second) return make_pair_expected_from_error<value_type> (f.first, f.second.error());

    auto m = matchedString("..", f.first, e);
    if (!m.second) return make_pair_expected_from_error<value_type> (m.first, m.second.error());
    //auto l = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
    auto l = ::NumGet<char_type, iter_type>().template get<Num> (m.first, e, ios);
    if (!l.second) return make_pair_expected_from_error<value_type> (l.first, l.second.error());

    value_type tmp = std::make_pair(*f.second, *l.second);
    return make_pair_expected<std::ios_base::iostate> (l.first, tmp);
  }

  template <typename Num>
  pair_expected<iter_type, std::pair<Num, Num>, std::ios_base::iostate> get2(InputIterator s, InputIterator e,
      std::ios_base& ios) const
  {
    using namespace boost;
    using namespace boost::monads;
    typedef std::pair<Num, Num> value_type;

    //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
    auto f = ::NumGet<char_type, iter_type>().template get<Num> (s, e, ios);
auto  m = mbind(f, [e](std::pair<iter_type,Num> f)
      {
        return matchedString("..", f.first, e);
      });
  auto l = mbind(m, [&ios, e](std::pair<iter_type, Num> m)
      {
        //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
        return ::NumGet<char_type, iter_type>().template get<Num> (m.first, e, ios);
      });
  return mbind(l, [f](std::pair<iter_type,Num> l)
      {
        value_type tmp(*f.second, l.second);
        return make_pair_expected<std::ios_base::iostate>(l.first, tmp);
      });

}

template <typename Num>
pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get3(InputIterator s,
    InputIterator e, std::ios_base& ios) const
{
  using namespace boost;
  using namespace boost::monads;
  typedef std::pair<Num, Num> value_type;

  //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
  auto f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);

  return make_monad(std::move(f))
  & [e](std::pair<iter_type,Num> f)
  {

    return matchedString("..", f.first, e);

  }& [&ios, e](std::pair<iter_type,Num> m)
  {

    //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
    return ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);

  }& [f](std::pair<iter_type,Num> l)
  {

    value_type tmp(*f.second, l.second);
    return make_pair_expected<std::ios_base::iostate>(l.first, tmp);

  };
}

template <typename Num>
pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get4(InputIterator s,
    InputIterator e, std::ios_base& ios) const
{
  using namespace boost;
  using namespace boost::monads;
  typedef std::pair<Num, Num> value_type;

  //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
  auto f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);

  return make_monad(std::move(f))
  .mbind([e](std::pair<iter_type,Num> f)
      {

        return matchedString("..", f.first, e);

      }).mbind( [&ios, e](std::pair<iter_type,Num> m)
      {

        //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
        return ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);

      }).mbind( [f](std::pair<iter_type,Num> l)
      {

        value_type tmp(*f.second, l.second);
        return make_pair_expected<std::ios_base::iostate>(l.first, tmp);

      });
}
template <typename Num>
pair_expected<iter_type, std::pair<Num,Num>, std::ios_base::iostate> get5(InputIterator s,
    InputIterator e, std::ios_base& ios) const
{
  using namespace boost;
  typedef std::pair<Num, Num> value_type;

  //auto  f = std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(s, e, ios);
  auto f = ::NumGet<char_type, iter_type>().template get<Num>(s, e, ios);

  return f
  & [e](std::pair<iter_type,Num> f)
  {

    return matchedString("..", f.first, e);

  }& [&ios, e](std::pair<iter_type,Num> m)
  {

    //return std::use_facet< ::NumGet<char_type, iter_type> >(ios.getloc()).template get<Num>(m.first, e, ios);
    return ::NumGet<char_type, iter_type>().template get<Num>(m.first, e, ios);

  }& [f](std::pair<iter_type,Num> l)
  {

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
    if (!x.second)
    {
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
    if (!x.second)
    {
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
    if (!x.second)
    {
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
    if (!x.second)
    {
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
    if (!x.second)
    {
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
    if (!x.second)
    {
      std::cout << x.second.error() << std::endl;
      return 1;
    }

    std::cout << x.second->first << ".." << x.second->second << std::endl;
  }

  return 0;
}

