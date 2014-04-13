// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_META_HPP
#define BOOST_FUNCTIONAL_META_HPP

#include <type_traits>

namespace boost
{
  namespace functional
  {
    // invokes a metafunction with args
    template<class F, class... Args>
    using apply = typename F::template type<Args...>;

    // transforms a template class into a metafunction, optionally partially curries it
    template <template <class ...> class F, class... Args>
    struct liftx
    {
      template<class... Args2>
      using type = F<Args..., Args2...>;
    };
    template <template <class ...> class F>
    struct lift
    {
      template<class... Args>
      using type = F<Args...>;
    };

    template <class M, class T>
    struct bind
    {
      typedef typename M::template bind<T>::type type;
    };

    // curry first N args of metafunction
//    template<class F, class... ArgsN>
//    struct curryl
//    {
//      template<class... ArgsK>
//      using type = apply<F, ArgsN..., ArgsK...>;
//    };
  }
}

#endif // BOOST_FUNCTIONAL_META__HPP
