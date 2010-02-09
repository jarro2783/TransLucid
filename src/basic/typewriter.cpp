#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <iconv.h>

std::string
utf32_to_utf8(const std::u32string& s) {
  const size_t buffer_size = 2000;
  if (s.size() > buffer_size)
  {
    return std::string("string too big");
  }
  iconv_t id = iconv_open("UTF-8", "UTF-32");

  size_t inSize = s.size();
  size_t outSize = buffer_size;
  char out[buffer_size];
  char in[buffer_size*4];
  memcpy(in, s.c_str(), s.size()*sizeof(char32_t));

  char* outp = out;
  char* inp = in;

  while (inSize > 0) {
    size_t r = iconv(id, &inp, &inSize, &outp, &outSize);
    if (r == (size_t)-1)
    {
      //std::cerr << "iconv failed: " << errno << std::endl;
      perror("iconv failed: ");
      inSize = 0;
    }
  }

  return std::string(out);
}

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

  std::cout << utf32_to_utf8(s) << std::endl;
  delete h;

  h = translator.translate_expr(L"\'é\'");
  v = (*h)(TL::Tuple());
  //std::cout << "char value = " << v.first.value<TL::Char>().value() << std::endl;
  s.clear();
  s += v.first.value<TL::Char>().value();
  std::cout << utf32_to_utf8(s) << std::endl;
  return 0;
}
