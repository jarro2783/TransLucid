#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>

namespace TL = TransLucid;

int main(int argc, char *argv[])
{
  TL::Translator translator;
  TL::HD* e = translator.translate_expr(L"42");

  TL::TaggedValue v = (*e)(TL::Tuple());
  std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;
  return 0;
}
