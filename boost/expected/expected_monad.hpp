// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_EXPECTED_MONAD_HPP
#define BOOST_EXPECTED_EXPECTED_MONAD_HPP

#include <boost/expected/expected_like_monad.hpp>
#include <boost/expected/expected.hpp>

namespace boost
{
  namespace monads
  {

    template <class T, class E>
    struct is_monad<expected<T,E> > : std::true_type {};

    template <class T, class E>
    struct functor_category<expected<T,E> > {
      typedef category::expected_like type;
    };
    template <class T, class E>
    struct monad_category<expected<T,E> > {
      typedef category::expected_like type;
    };
    template <class T, class E>
    struct monad_error_category<expected<T,E> > {
      typedef category::expected_like type;
    };

    template <class T, class E>
    struct unexpected_traits<expected<T,E> > {
      using type = unexpected<E>;
      static constexpr unexpected<E> get_unexpected(expected<T, E> const& e) { return e.get_unexpected(); }
    };


  }
}

#undef REQUIRES
#undef T_REQUIRES

#endif // BOOST_EXPECTED_EXPECTED_MONAD_HPP
