#include <tl/hyperdaton.hpp>

int main(int argc, char *argv[])
{
  TransLucid::ArrayNHD<int, 3> a({5,6,7});

  auto c = a.get_array();
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
        //std::cout << c[i][j][k] << " ";
        //std::cout << a.get(i, j, k) << " ";
        std::cout << a[i][j][k] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
  }

  std::cout << std::endl;

  //std::cout << a.get(1, 1, 1) << std::endl;

  return 0;
}
