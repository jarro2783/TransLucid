#ifndef TL_GMPXX_FWD_HPP_INCLUDED
#define TL_GMPXX_FWD_HPP_INCLUDED

#include <gmp.h>

template <typename T, typename U>
class __gmp_expr;

template <> class __gmp_expr<mpz_t, mpz_t>;

typedef __gmp_expr<mpz_t, mpz_t> mpz_class;

template <> class __gmp_expr<mpf_t, mpf_t>;

typedef __gmp_expr<mpf_t, mpf_t> mpf_class;

namespace std
{
  template <>
  struct hash<mpz_class>
  {
    size_t
    operator()(const mpz_class& i) const;
  };
}

#endif
