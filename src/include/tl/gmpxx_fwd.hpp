#ifndef TL_GMPXX_FWD_HPP_INCLUDED
#define TL_GMPXX_FWD_HPP_INCLUDED

#include <gmp.h>

template <typename T, typename U>
class __gmp_expr;

template <> class __gmp_expr<mpz_t, mpz_t>;

typedef __gmp_expr<mpz_t, mpz_t> mpz_class;

#endif
