#include <tl/parser_iterator.hpp>

using namespace TransLucid::Parser;

int
main(int argc, char* argv[])
{
  std::cin >> std::noskipws;
  U32Iterator iter(
    makeUTF8Iterator(std::istream_iterator<char>(std::cin)),
    makeUTF8Iterator(std::istream_iterator<char>())
  );

  while (iter != U32Iterator())
  {
    char32_t c = *iter;
    ++iter;
    //this should print because we haven't actually dereferenced
    //the iterator even though we incremented it
    std::cout << c << std::endl;
  }

  return 0;
}
