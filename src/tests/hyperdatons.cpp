#include <tl/types/intmp.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/system.hpp>

int main(int argc, char *argv[])
{
  namespace Intmp = TransLucid::Types::Intmp;

  TransLucid::System s;
  TransLucid::ArrayNHD<mpz_class, 3> a
    (
      {5,6,7},
      {Intmp::create(0), Intmp::create(1), Intmp::create(2)},
      s,
      s,
      U"intmp"
    );

  a.variance();

  auto& c = a.get_array();
  c[1][2][3] = 5;

  auto x = c[1];
  auto y = x[2];
  auto z = y[3];
  std::cout << z << std::endl;

  for (int i = 0; i != 5; ++i)
  {
    for (int j = 0; j != 6; ++j)
    {
      for (int k = 0; k != 7; ++k)
      {
        c[i][j][k] = i * j * k;
      }
    }
  }

  for (int i = 0; i != 5; ++i)
  {
    for (int j = 0; j != 6; ++j)
    {
      for (int k = 0; k != 7; ++k)
      {
        std::cout << a[i][j][k] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }

  auto val = a.get(1, 1, 1);
  std::cout << val << std::endl;

  return 0;
}
