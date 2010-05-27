#include <tl/translator.hpp>

using namespace TransLucid;

int main(int argc, char* argv[])
{
  HD* e = 0;
  try {
    Translator t;

    t.parse_header
    (
      U"dimension ustring<n>;;"
      U"infixl ustring<-> ustring<operator-> 5;;"
      U"infixl ustring<*> ustring<operator*> 10;;"
      U"library ustring<int>;;"
    );

    t.translate_and_add_equation_set
    (
      U"fact | [n:0] = 1;;"
      U"fact = #n * (fact @ [n:#n-1]);;"
    );

    e = t.translate_expr(U"fact @ [n:20]");

    TaggedConstant result = (*e)(Tuple());

    std::cout << result.first.value<Intmp>().value() << std::endl;
  } catch (const char* c) {
    std::cerr << "Terminated with exception: " << c << std::endl;
  } catch (std::exception& e) {
    std::cerr << "Terminated with exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Terminated with unknown exception" << std::endl;
  }

  delete e;

  return 0;
}
