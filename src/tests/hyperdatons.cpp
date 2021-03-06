#include <tl/types/intmp.hpp>
#include <tl/types/range.hpp>
#include <tl/hyperdatons/arrayhd.hpp>
#include <tl/system.hpp>

int main(int argc, char *argv[])
{
  namespace TL = TransLucid;
  namespace Intmp = TL::Types::Intmp;

  TL::Constant dim0, dim1, dim2;
  dim0 = Intmp::create(0);
  dim1 = Intmp::create(1);
  dim2 = Intmp::create(2);

  TL::System s;
  TL::ArrayNHD<mpz_class, 3> array
    (
      {5,6,7},
      {dim0, dim1, dim2},
      s,
      static_cast<TL::Constant (*)(const mpz_class&)>
        (&TL::Types::Intmp::create),
      &TL::Types::Intmp::get
    );

  TL::Tuple var = array.variance();

  mpz_class zero = 0, a = 4, b = 5, c = 6;
  TL::tuple_t expected
  {
    {s.getDimensionIndex(dim0), 
      TL::Types::Range::create(TL::Range(&zero, &a))},
    {s.getDimensionIndex(dim1), 
      TL::Types::Range::create(TL::Range(&zero, &b))},
    {s.getDimensionIndex(dim2), 
      TL::Types::Range::create(TL::Range(&zero, &c))}
  };

  std::cerr << "variance: ";
  for (auto v : var)
  {
    std::cerr << v.first << ": ";
    TL::Types::Range::get(v.second).print(std::cout);
    std::cerr << ", ";
  }
  std::cerr << std::endl;

  std::cerr << "expected: ";
  for (auto v : expected)
  {
    std::cerr << v.first << ": ";
    TL::Types::Range::get(v.second).print(std::cout);
    std::cerr << ", ";
  }
  std::cerr << std::endl;

  assert(var == TL::Tuple(expected));

  auto& d = array.get_array();
  d[1][2][3] = 5;

  auto x = d[1];
  auto y = x[2];
  auto z = y[3];
  std::cout << z << std::endl;

  for (int i = 0; i != 5; ++i)
  {
    for (int j = 0; j != 6; ++j)
    {
      for (int k = 0; k != 7; ++k)
      {
        d[i][j][k] = i * j * k;
      }
    }
  }

  for (int i = 0; i != 5; ++i)
  {
    for (int j = 0; j != 6; ++j)
    {
      for (int k = 0; k != 7; ++k)
      {
        std::cout << array[i][j][k] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }

  auto val = array.get(1, 1, 1);
  std::cout << val << std::endl;

  return 0;
}
