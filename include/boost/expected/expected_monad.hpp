// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_MONAD_HPP

#include <boost/functional/monads/categories/valued_and_errored.hpp>
#include <boost/functional/monads/monad_error.hpp>
#include <boost/expected/expected.hpp>
#include <boost/expected/unexpected.hpp>
#include <boost/mpl/identity.hpp>
#include <type_traits>

namespace boost
{
  namespace functional
  {
    namespace valued
    {
      template <class E, class T>
      struct value_traits<expected<E, T>>
      {
        constexpr static bool value = true;

        template <class M>
        static constexpr bool has_value(M&& m) { return bool(m); }

        template <class M>
        static constexpr auto deref(M&& m) -> decltype(*m) { return *m; }

        template <class M>
        static constexpr value_type_t<M> get_value(M&& m) { return m.value(); };
      };
    }
    namespace functor
    {
      template <class T, class E>
      struct functor_category<expected<E, T> > : mpl::identity<category::valued_and_errored> {};
    }
    namespace monad
    {
      template <class T, class E>
      struct is_monad<expected<E, T> > : std::true_type {};

      //    template <class T, class E>
      //    struct monad_category<expected<E,T> > : mpl::identity<category::valued_and_errored> { };

//      template <class T1, class E1>
//      struct monad_traits<expected<E1, T1>>
//      {
//        template <class M, class T>
//        static apply<M, T> make(T&& v)
//        {
//          return apply<M, T>(std::forward<T>(v));
//        }
//
//        template <class M, class F>
//        static auto
//        mbind(M&& m, F&& f) -> decltype(m.mbind(std::forward<F>(f)))
//        {
//          return m.mbind(std::forward<F>(f));
//        }
//      };
    }
    namespace monad_error
    {
      static_assert(std::is_same<
          monad_error_category_t<expected<std::exception_ptr,int>>,
          expected<std::exception_ptr,int>>::value, "");

      template <class T1, class E1>
      struct monad_error_traits<expected<E1,T1> >
      {
        template <class M>
        static constexpr auto value(M&& m) -> decltype(m.value())
        { return m.value();};

        template <class M, class E>
        static auto make_error(E&& e) -> decltype(make_unexpected(std::forward<E>(e)))
        {
          return make_unexpected(std::forward<E>(e));
        }

        template <class M, class F>
        static M catch_error(M&& m, F&& f)
        {
          return m.recover(std::forward<F>(f));
        }
      };
    }
  }
}

#endif // BOOST_EXPECTED_EXPECTED_MONAD_HPP
