// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_EXPECTED_MONADS_ALGORITHMS_FIRST_UNEXPECTED_HPP
#define BOOST_EXPECTED_MONADS_ALGORITHMS_FIRST_UNEXPECTED_HPP

#include <boost/functional/monads/errored.hpp>
#include <boost/functional/monads/valued.hpp>

namespace boost
{
namespace functional
{
namespace errored
{
  using namespace ::boost::functional::valued;

  template< class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
  BOOST_CONSTEXPR unexpected_type_t<M> first_unexpected( M&& m )
  {
    return get_unexpected(std::forward<M>(m));
  }
  template< class M1, class ...Ms
    , class TraitsValued = unexpected_traits<unexpected_category_t<decay_t<M1> > >
    , class TraitsUnexpected = unexpected_traits<unexpected_category_t<decay_t<M1> > > >
  BOOST_CONSTEXPR unexpected_type_t<M1> first_unexpected( M1&& m1, Ms&& ...ms )
  {
    return has_value(std::forward<M1>(m1))
        ? first_unexpected( std::forward<Ms>(ms)... )
            : get_unexpected(std::forward<M1>(m1)) ;
  }

}
}
}


#endif // BOOST_EXPECTED_MONADS_ALGORITHMS_FIRST_UNEXPECTED_HPP
