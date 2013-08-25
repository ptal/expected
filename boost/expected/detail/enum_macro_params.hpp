// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013 Pierre Talbot

# ifndef BOOST_PREPROCESSOR_REPETITION_ENUM_MACRO_PARAMS_HPP
# define BOOST_PREPROCESSOR_REPETITION_ENUM_MACRO_PARAMS_HPP
#
# include <boost/preprocessor/cat.hpp>
# include <boost/preprocessor/config/config.hpp>
# include <boost/preprocessor/punctuation/comma_if.hpp>
# include <boost/preprocessor/repetition/repeat.hpp>
# include <boost/preprocessor/tuple/elem.hpp>
# include <boost/preprocessor/tuple/rem.hpp>
#
# /* BOOST_PP_ENUM_MACRO_PARAMS */
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_EDG()
#    define BOOST_PP_ENUM_MACRO_PARAMS(count, macro, param) BOOST_PP_REPEAT(count, BOOST_PP_ENUM_MACRO_PARAMS_M, (macro, param))
# else
#    define BOOST_PP_ENUM_MACRO_PARAMS(count, macro, param) BOOST_PP_ENUM_MACRO_PARAMS_I(count, macro, param)
#    define BOOST_PP_ENUM_MACRO_PARAMS_I(count, macro, param) BOOST_PP_REPEAT(count, BOOST_PP_ENUM_MACRO_PARAMS_M, (macro, param))
# endif
#
# if BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_STRICT()
#    define BOOST_PP_ENUM_MACRO_PARAMS_M(z, n, pp) BOOST_PP_ENUM_MACRO_PARAMS_M_IM(z, n, BOOST_PP_TUPLE_REM_2 pp)
#    define BOOST_PP_ENUM_MACRO_PARAMS_M_IM(z, n, im) BOOST_PP_ENUM_MACRO_PARAMS_M_I(z, n, im)
# else
#    define BOOST_PP_ENUM_MACRO_PARAMS_M(z, n, pp) BOOST_PP_ENUM_MACRO_PARAMS_M_I(z, n, BOOST_PP_TUPLE_ELEM(2, 0, pp), BOOST_PP_TUPLE_ELEM(2, 1, pp))
# endif
#
# if ~BOOST_PP_CONFIG_FLAGS() & BOOST_PP_CONFIG_MSVC()
#    define BOOST_PP_ENUM_MACRO_PARAMS_M_I(z, n, macro, param) BOOST_PP_ENUM_MACRO_PARAMS_M_II(z, n, macro, param)
#    define BOOST_PP_ENUM_MACRO_PARAMS_M_II(z, n, macro, param) BOOST_PP_COMMA_IF(n) macro(param ## n)
# else
#    define BOOST_PP_ENUM_MACRO_PARAMS_M_I(z, n, macro, param) BOOST_PP_COMMA_IF(n) macro(BOOST_PP_CAT(param, n))
# endif
#
# endif
