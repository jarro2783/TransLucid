#include <gmpxx.h>

namespace TransLucid
{
  namespace Parser
  {
    mpq_class
    init_mpq
    (
      const std::wstring::const_iterator& begin, 
      const std::wstring::const_iterator& end,
      int base
    );
  }
}
