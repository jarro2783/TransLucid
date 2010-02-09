#ifndef COMPILED_CLASSES_HPP_INCLUDED
#define COMPILED_CLASSES_HPP_INCLUDED

#include <string>

#if 0
#include <tl/types.hpp>

namespace TransLucid
{
  typedef <typename T, K N>
  class Constant
  {
    public:

    typedef T RawType;

    RawType
    operator()(Tuple& k)
    {
      return N;
    }

    private:
  };

  typedef <typename T, int32_t N>
  class Constant
  {
    public:

    T
    operator()(Tuple& k)
    {
      return N;
    }

    private:
  };
#endif

extern std::string x;

  template <std::string* N>
  class F
  {
  };

  int
  main()
  {
    //std::string x;
    F<&x> y;
  }

  //int32<3> + int32<4>;;
  //
  // Constant<int32_t, 3>()() + Constant<int32_t, 4>()()

  //Constant<int32_t>(4)() + Constant<int32>(3)();

  //Constant<Glib::ustring,"h">

#if 0
}
#endif

#endif // COMPILED_CLASSES_HPP_INCLUDED
