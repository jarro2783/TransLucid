#include <tl/translator.hpp>

namespace TL = TransLucid;

int main(int argc, char* argv[])
{
  TL::Translator t;

  t.parse_header
  (
    U"dimension ustring<n>;;"
  );

  t.translate_and_add_equation_set
  (
    U"fact | [n:0] = 1;;"
    U"fact = #n * (fact @ [n:#n-1]);;"
  );

  TL::HD* e = t.translate_expr(U"fact @ [n:20]");

  TL::TaggedConstant result = (*e)(TL::Tuple());

  std::cout << result.first.value<TL::Intmp>().value() << std::endl;

  delete e;

  return 0;
}
