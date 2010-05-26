#include <tl/translator.hpp>

namespace TL = TransLucid;

int main(int argc, char* argv[])
{
  TL::Translator t;

  t.translate_and_add_equation_set
  (
    U"fact | [n:0] = 1;;"
    U"fact = #n * (fact @ [n:#n-1]);;"
  );
  return 0;
}
