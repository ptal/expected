// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2014 Vicente J. Botet Escriba

#ifndef BOOST_FUNCTIONAL_MONAD_HPP
#define BOOST_FUNCTIONAL_MONAD_HPP

#include <boost/config.hpp>
#include <boost/functional/type_traits_t.hpp>
#include <exception>
#include <utility>
#include <type_traits>

namespace boost
{
  namespace monads
  {

    template <class M>
    struct is_monad : std::false_type {};

    template <class M>
    struct unexpected_category {
      typedef M type;
    };

    template <class M>
    using unexpected_category_t = typename unexpected_category<M>::type;

    template <class T>
    struct unexpected_traits;

    template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > >>
    using unexpected_type_t = typename Traits::template type<M>;

    template <class M, class Traits = unexpected_traits<unexpected_category_t<decay_t<M> > > >
    static constexpr unexpected_type_t<decay_t<M>>
    get_unexpected(M&& e)
    {
      return Traits::get_unexpected(std::forward<M>(e));
    }


    template <class M>
    struct functor_category {
      typedef M type;
    };

    template <class M>
    using functor_category_t = typename functor_category<M>::type;

    template <class M, class T>
    struct bind
    {
      typedef typename M::template bind<T>::type type;
    };

    template <class Mo>
    struct functor_traits {

      template <class F, class M0, class ...M, class FR = decltype( std::declval<F>()(*std::declval<M0>(), *std::declval<M>()...) )>
      static auto
      when_all_valued(F&& f, M0&& m0, M&& ...ms) -> typename bind<decay_t<M0>, FR>::type
      {
        return M0::when_all_valued(std::forward<F>(f), std::forward<M0>(m0), std::forward<M>(ms)...);
      }
    };
    template <class M>
    struct monad_category {
      typedef M type;
    };

    template <class M>
    using monad_category_t = typename monad_category<M>::type;

    template <class Mo>
    struct monad_traits {

//      template <class M>
//      struct value_type
//      {
//        typedef typename M::value_type type;
//      };

//      template <class M, class F>
//      struct when_ready_result
//      {
//        typedef typename bind<M, typename std::result_of<F(M)>::type>::type type;
//      };

//      template <class M, class F>
//      struct when_valued_result
//      {
//        typedef typename bind<M, typename std::result_of<F(typename value_type<M>::type)>::type>::type type;
//      };
//
      template <class M, class T>
      static M make(T&& v)
      {
        return M(std::forward<T>(v));
      }

      template <class M, class F>
      static auto
      when_ready(M&& m, F&& f) -> decltype(m.when_ready(std::forward<F>(f)))
      {
        m.when_ready(std::forward<F>(f));
      }

      template <class M, class F>
      static auto
      when_valued(M&& m, F&& f) -> decltype(m.when_valued(std::forward<F>(f)))
      {
        return m.when_valued(std::forward<F>(f));
      }

    };

    template <class M, class T, class Traits = monad_traits<monad_category_t<decay_t<M>> > >
    M make(T&& v)
    {
      return Traits::make(std::forward<T>(v));
    }

    template <template <class ...> class M, class T, class Traits = monad_traits<monad_category_t<M<T> > >>
    M<T> make(T&& v)
    {
      return Traits::make(std::forward<T>(v));
    }

    template <class M, class F, class Traits = monad_traits<monad_category_t<decay_t<M> > > >
    auto
    when_ready(M&& m, F&& f) -> decltype(Traits::when_ready(std::forward<M>(m), std::forward<F>(f)))
    {
      return Traits::when_ready(std::forward<M>(m), std::forward<F>(f));
    }

    template <class M, class F, class Traits = monad_traits<monad_category_t<decay_t<M> > > >
    auto
    when_valued(M&& m, F&& f) -> decltype(Traits::when_valued(std::forward<M>(m), std::forward<F>(f)))
    {
      return Traits::when_valued(std::forward<M>(m), std::forward<F>(f));
    }

    template <class F, class M0, class ...M, class Traits = functor_traits<functor_category_t<decay_t<M0> > > >
    auto
    when_all_valued(F&& f, M0&& m0, M&& ...m)
    -> decltype(Traits::when_all_valued(std::forward<F>(f), std::forward<M0>(m0), std::forward<M>(m)...))
    {
      return Traits::when_all_valued(std::forward<F>(f),std::forward<M0>(m0), std::forward<M>(m)...);
    }

    template <class M>
    class monad;

    template <class M>
    monad<decay_t<M>> make_monad(M&& v)
    {
      return monad<decay_t<M>>(std::forward<M>(v));
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

      template <class F, class R = decltype(make_monad(monads::when_ready(std::declval<M>(), std::declval<F>())))>
      auto when_ready(F&& f) -> R
      {
        return make_monad(monads::when_ready(m, std::forward<F>(f)));
      }

      template <class F, class R = decltype(make_monad(monads::when_valued(std::declval<M>(), std::declval<F>())))>
      auto when_valued(F&& f) -> R
      {
        return make_monad(monads::when_valued(m, std::forward<F>(f)));
      }
    };

    template <class T, class F>
    auto operator>>(monad<T>&& m, F&& f)
    -> decltype(m.when_ready(std::forward<F>(f)))
    {
      return m.when_ready(std::forward<F>(f));
    }

    template <class T, class F>
    auto operator&(monad<T>&& x, F&& f)
    -> decltype(x.when_valued(std::forward<F>(f)))
    {
      return x.when_valued(std::forward<F>(f));
    }

    template <class M>
    struct monad_error_category : monad_category<M> {};

    template <class M>
    using monad_error_category_t = typename monad_error_category<M>::type;

    template <class Mo>
    struct monad_error_traits : monad_traits<Mo> {

      template <class M, class F>
      M when_unexpected(M&& m, F&& f)
      {
        return m.when_unexpected(std::forward<F>(f));
      }

    };

    template <class M, class F, class Traits = monad_error_traits<monad_error_category_t<decay_t<M> > > >
    static M when_unexpected(M&& m, F&& f)
    {
      return Traits::when_unexpected(std::forward<M>(m), std::forward<F>(f));
    }

    // monad_error
    template <class M>
    class monad_error;

    template <class M>
    monad_error<decay_t<M>> make_monad_error(M&& v)
    {
      return monad_error<decay_t<M>>(std::forward<M>(v));
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
      auto when_ready(F&& f) const
      -> decltype(make_monad_error(monads::when_ready(this->m, std::forward<F>(f))))
      {
        return make_monad_error(monads::when_ready(this->m, std::forward<F>(f)));
      }

      template <class F>
      auto when_valued(F&& f) const
      -> decltype(make_monad_error(monads::when_valued(this->m, std::forward<F>(f))))
      {
        return make_monad_error(monads::when_valued(this->m, std::forward<F>(f)));
      }

      template <class F>
      auto when_unexpected(F&& f) const
      -> decltype(make_monad_error(monads::when_unexpected(this->m, std::forward<F>(f))))
      {
        return make_monad_error(monads::when_unexpected(this->m, std::forward<F>(f)));
      }
    };

    template <class T, class F>
    auto operator>>(monad_error<T>&& m, F&& f)
    -> decltype(m.when_ready(std::forward<F>(f)))
    {
      return m.when_ready(std::forward<F>(f));
    }

    template <class T, class F>
    auto operator&(monad_error<T>&& x, F&& f)
    -> decltype(x.when_valued(std::forward<F>(f)))
    {
      return x.when_valued(std::forward<F>(f));
    }

    template <class T, class F>
    auto operator|(monad_error<T>&& m, F&& f)
    -> decltype(m.when_unexpected(std::forward<F>(f)))
    {
      return m.when_unexpected(std::forward<F>(f));
    }

  }
}

#endif // BOOST_FUNCTIONAL_MONAD_HPP
