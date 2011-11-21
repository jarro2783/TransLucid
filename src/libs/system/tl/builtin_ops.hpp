namespace TransLucid
{
  namespace BuiltinOps
  {
    Constant
    mpz_plus(const Constant& a, const Constant& b);

    Constant
    mpz_minus(const Constant& a, const Constant& b);

    Constant
    mpz_times(const Constant& a, const Constant& b);

    Constant
    mpz_divide(const Constant& a, const Constant& b);

    Constant
    mpz_modulus(const Constant& a, const Constant& b);

    Constant
    mpz_lte(const Constant& a, const Constant& b);

    Constant
    mpz_lt(const Constant& a, const Constant& b);

    Constant
    mpz_gte(const Constant& a, const Constant& b);

    Constant
    mpz_gt(const Constant& a, const Constant& b);

    Constant
    mpz_eq(const Constant& a, const Constant& b);

    Constant
    mpz_ne(const Constant& a, const Constant& b);

    Constant
    bool_eq(const Constant& a, const Constant& b);

    Constant
    bool_ne(const Constant& a, const Constant& b);

    Constant
    bool_and(const Constant& a, const Constant& b);

    Constant
    bool_or(const Constant& a, const Constant& b);

    Constant
    ustring_eq(const Constant& a, const Constant& b);

    Constant
    ustring_ne(const Constant& a, const Constant& b);

    Constant
    ustring_plus(const Constant& a, const Constant& b);
  }
}
