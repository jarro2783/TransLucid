#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <tl/utility.hpp>

namespace TL = TransLucid;

int main(int argc, char *argv[])
{
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

  delete h;
  h = translator.translate_expr(L"spdim");
  v = (*h)(TL::Tuple());
  std::cout << v.first.value<TL::Special>().value() << std::endl;

  h = translator.translate_expr(L"0X10");
  v = (*h)(TL::Tuple());
  std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;
  return 0;
}
