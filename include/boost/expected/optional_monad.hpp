// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_OPTIONAL_MONAD_HPP
#define BOOST_EXPECTED_OPTIONAL_MONAD_HPP


#include <boost/functional/meta.hpp>
#include <boost/optional.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/functional/monads/errored.hpp>
#include <boost/functional/monads/functor.hpp>
#include <boost/functional/monads/categories/errored.hpp>
#include <boost/functional/monads/monad.hpp>
#include <boost/functional/monads/monad_error.hpp>
#include <type_traits>

namespace boost
{
  using optional_monad = functional::lift<optional>;

  namespace functional
  {
    namespace rebindable
    {
      template <class T>
      struct rebindable_traits<optional<T>> : std::true_type
      {
        template <class M>
        using value_type = typename M::value_type;

        template <class M, class U>
        using rebind = optional<U>;

      };
    }
    namespace valued
    {
      template <class T>
      struct value_traits<optional<T>> : std::true_type
      {
        template <class M>
        static constexpr bool has_value(M&& m) { return bool(m); }

        template <class M>
        static constexpr auto deref(M&& m) -> decltype(*m) { return *m; }

        template <class M>
        static constexpr value_type_t<M> get_value(M&& m) { return m.value(); };
      };
    }
    namespace errored
    {
      template <class T>
      struct unexpected_traits<optional<T> > : std::true_type
      {
        template< class M >
        using unexpected_type_type = none_t;

        template <class M, class E>
        static M make_error(E&&)
        { return none; }

        static constexpr none_t get_unexpected(optional<T> const& )
        { return none; }

        template< class M >
        static constexpr none_t error(M && m)
        { return none; }

      };
    }
    namespace functor
    {
      template <class T>
      struct functor_category<optional<T> > : mpl::identity<category::errored> {};
    }
    namespace monad
    {

      template <class T>
      struct is_monad<optional<T> > : std::true_type {};

      template <class T>
      struct monad_category<optional<T> > : mpl::identity<category::errored> {};

    }
    namespace monad_error
    {
      using namespace ::boost::functional::errored;

      template <class T>
      struct monad_error_traits<optional<T> > : std::true_type
      {
        template <class M>
        static constexpr auto value(M&& m) -> decltype(m.value())
        { return m.value();}

        template <class M, class E>
        static auto make_error(E&&) -> decltype(none)
        {
          return none;
        }

        // f : E -> T
        // todo complete with the other prototypes
        // f : E -> void
        // f : E -> M
        template <class M, class F, class FR = decltype( std::declval<F>()( none ) ) >
        static BOOST_CONSTEXPR M
        catch_error(M&& m, F&& f)
        {
          typedef rebind<decay_t<M>, FR> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
          if(! has_value(m))
          {
            result_type(f(none));
          }
          return deref(m);
#else
          return (! has_value(m)
              ? result_type(f(none))
              : deref(m)
          );
#endif
        }
      };
      template <>
      struct monad_error_traits<optional_monad > : std::true_type
      {
        template <class M>
        static constexpr auto value(M&& m) -> decltype(m.value())
        { return m.value();};

        template <class M, class E>
        static auto make_error(E&&) -> decltype(none)
        {
          return none;
        }

        // f : E -> T
        // todo complete with the other prototypes
        // f : E -> void
        // f : E -> M
        template <class M, class F, class FR = decltype( std::declval<F>()( none ) ) >
        static BOOST_CONSTEXPR M
        catch_error(M&& m, F&& f)
        {
          typedef rebind<decay_t<M>, FR> result_type;
#if ! defined BOOST_NO_CXX14_RELAXED_CONSTEXPR
          if(! has_value(m))
          {
            result_type(f(none));
          }
          return deref(m);
#else
          return (! has_value(m)
              ? result_type(f(none))
              : deref(m)
          );
#endif
        }
      };
    }
  }
}

#endif // BOOST_EXPECTED_OPTIONAL_MONAD_HPP
