#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <tl/utility.hpp>

namespace TL = TransLucid;

int main(int argc, char *argv[])
{
  std::cout << "in unicode é looks like" << std::endl;
  std::u32string echar = U"é";
  std::cout << echar[0] << std::endl;

  TL::Translator translator;
  TL::HD* h = translator.translate_expr(L"42");

  TL::TaggedValue v = (*h)(TL::Tuple());
  std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;

  delete h;

  h = translator.translate_expr(L"«hello é world»");

  v = (*h)(TL::Tuple());
  std::u32string s = v.first.value<TL::String>().value();

  std::cout << TL::utf32_to_utf8(s) << std::endl;
  delete h;

  h = translator.translate_expr(L"\'è\'");
  v = (*h)(TL::Tuple());
  //std::cout << "char value = " << v.first.value<TL::Char>().value() << std::endl;
  s.clear();
  s += v.first.value<TL::Char>().value();
  std::cout << TL::utf32_to_utf8(s) << std::endl;
  return 0;
}
