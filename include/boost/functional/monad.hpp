// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_MONAD_HPP
#define BOOST_FUNCTIONAL_MONAD_HPP

//#include <boost/config.hpp>
//#include <boost/functional/valued.hpp>
//#include <boost/functional/errored.hpp>
//#include <boost/functional/functor.hpp>
#include <boost/functional/type_traits_t.hpp>
//#include <exception>
#include <utility>
//#include <type_traits>

namespace boost
{
  namespace monads
  {

    template <class M>
    struct is_monad : std::false_type {};

    template <class M>
    struct monad_category {
      typedef M type;
    };

    template <class M>
    using monad_category_t = typename monad_category<M>::type;

    template <class Mo>
    struct monad_traits {

      template <class M, class T>
      static M make(T&& v)
      {
        return M(std::forward<T>(v));
      }

      template <class M, class F>
      static auto
      then(M&& m, F&& f) -> decltype(m.then(std::forward<F>(f)))
      {
        m.then(std::forward<F>(f));
      }

      template <class M, class F>
      static auto
      mbind(M&& m, F&& f) -> decltype(m.mbind(std::forward<F>(f)))
      {
        return m.mbind(std::forward<F>(f));
      }

    };

    template <class M, class T, class Traits = monad_traits<monad_category_t<decay_t<M> > > >
    M make(T&& v)
    {
      return Traits::make(std::forward<T>(v));
    }

    template <template <class ...> class M, class T, class Traits = monad_traits<monad_category_t<M<T> > > >
    M<T> make(T&& v)
    {
      return Traits::make(std::forward<T>(v));
    }

    template <class M, class F, class Traits = monad_traits<monad_category_t<decay_t<M> > > >
    auto
    then(M&& m, F&& f) -> decltype(Traits::then(std::forward<M>(m), std::forward<F>(f)))
    {
      return Traits::then(std::forward<M>(m), std::forward<F>(f));
    }

    template <class M, class F, class Traits = monad_traits<monad_category_t<decay_t<M> > > >
    auto
    mbind(M&& m, F&& f) -> decltype(Traits::mbind(std::forward<M>(m), std::forward<F>(f)))
    {
      return Traits::mbind(std::forward<M>(m), std::forward<F>(f));
    }

    template <class M>
    class monad;

    template <class M>
    monad<decay_t<M> > make_monad(M&& v)
    {
      return monad<decay_t<M> >(std::forward<M>(v));
    }

    template <class M>
    class monad
    {
    protected:
      M m;
    public:
      monad(M&& v) : m(std::move(v))
      {}
      //    monad(monad const& x) : m(x.m)
        //    {}
        //    monad operator=(monad cons& x) {
        //      m = x.m;
        //      return *this;
        //    }

      monad& operator=(M&& v)
      {
        m = std::move(v);
      }
      monad(monad && x) : m(std::move(x.m))
      {}
      monad& operator=(monad&& x)
      {
        m = std::move(x.m);
        return *this;
      }

      operator M()
      { return std::move(m);}

      M release()
      { return std::move(m);}

      template <class F, class R = decltype(make_monad(monads::then(std::declval<M>(), std::declval<F>())))>
      auto then(F&& f) -> R
      {
        return make_monad(monads::then(m, std::forward<F>(f)));
      }

      template <class F, class R = decltype(make_monad(monads::mbind(std::declval<M>(), std::declval<F>())))>
      auto mbind(F&& f) -> R
      {
        return make_monad(monads::mbind(m, std::forward<F>(f)));
      }
    };

    template <class T, class F>
    auto operator>>(monad<T>&& m, F&& f)
    -> decltype(m.then(std::forward<F>(f)))
    {
      return m.then(std::forward<F>(f));
    }

    template <class T, class F>
    auto operator&(monad<T>&& x, F&& f)
    -> decltype(x.mbind(std::forward<F>(f)))
    {
      return x.mbind(std::forward<F>(f));
    }

  }
}

#endif // BOOST_FUNCTIONAL_MONAD_HPP
