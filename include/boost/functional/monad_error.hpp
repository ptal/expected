// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_MONAD_ERROR_HPP
#define BOOST_FUNCTIONAL_MONAD_ERROR_HPP

//#include <boost/config.hpp>
//#include <boost/functional/valued.hpp>
//#include <boost/functional/errored.hpp>
//#include <boost/functional/functor.hpp>
//#include <boost/functional/monad.hpp>
#include <boost/functional/type_traits_t.hpp>
//#include <exception>
#include <utility>
//#include <type_traits>

namespace boost
{
  namespace monads
  {
    template <class M>
    struct monad_error_category : monad_category<M> {};

    template <class M>
    using monad_error_category_t = typename monad_error_category<M>::type;

    template <class Mo>
    struct monad_error_traits : monad_traits<Mo> {

      template <class M, class F>
      M catch_error(M&& m, F&& f)
      {
        return m.catch_error(std::forward<F>(f));
      }

    };

    template <class M, class F, class Traits = monad_error_traits<monad_error_category_t<decay_t<M> > > >
    static M catch_error(M&& m, F&& f)
    {
      return Traits::catch_error(std::forward<M>(m), std::forward<F>(f));
    }

    // monad_error
    template <class M>
    class monad_error;

    template <class M>
    monad_error<decay_t<M> > make_monad_error(M&& v)
    {
      return monad_error<decay_t<M> >(std::forward<M>(v));
    }

    template <class M>
    class monad_error : public monad<M>
    {
    public:
      monad_error(M&& v) : monad<M>(std::forward(v))
      {}

      monad_error(monad_error&& x) : monad<M>(std::move(static_cast<monad<M>&>(x)))
      {}
      monad_error& operator=(monad_error const& x)
      {
        monad<M>::operator=(static_cast<const monad<M>&>(x));
        return *this;
      }
      monad_error& operator=(monad_error&& x)
      {
        monad<M>::operator=(std::move(static_cast<monad<M>&>(x)));
        return *this;
      }

      monad_error& operator=(M&& v)
      {
        monad<M>::operator=(std::forward<M>(v));
        return *this;
      }

      template <class F>
      auto then(F&& f) const
      -> decltype(make_monad_error(monads::then(this->m, std::forward<F>(f))))
      {
        return make_monad_error(monads::then(this->m, std::forward<F>(f)));
      }

      template <class F>
      auto mbind(F&& f) const
      -> decltype(make_monad_error(monads::mbind(this->m, std::forward<F>(f))))
      {
        return make_monad_error(monads::mbind(this->m, std::forward<F>(f)));
      }

      template <class F>
      auto catch_error(F&& f) const
      -> decltype(make_monad_error(monads::catch_error(this->m, std::forward<F>(f))))
      {
        return make_monad_error(monads::catch_error(this->m, std::forward<F>(f)));
      }
    };

    template <class T, class F>
    auto operator>>(monad_error<T>&& m, F&& f)
    -> decltype(m.then(std::forward<F>(f)))
    {
      return m.then(std::forward<F>(f));
    }

    template <class T, class F>
    auto operator&(monad_error<T>&& x, F&& f)
    -> decltype(x.mbind(std::forward<F>(f)))
    {
      return x.mbind(std::forward<F>(f));
    }

    template <class T, class F>
    auto operator|(monad_error<T>&& m, F&& f)
    -> decltype(m.catch_error(std::forward<F>(f)))
    {
      return m.catch_error(std::forward<F>(f));
    }

  }
}

#endif // BOOST_FUNCTIONAL_MONAD_ERROR_HPP
