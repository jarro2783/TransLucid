#include <tl/parser_fwd.hpp>

namespace TransLucid
{
  namespace TLCore
  {
    template <typename Iterator>
    class Grammar;

    class TLCore
    {
      public:
      TLCore();

      void verbose(bool v)
      {
        m_verbose = v;
      }

      void run();

      private:
      bool m_verbose;
      Grammar<Parser::string_type::const_iterator>* m_parser;
    };
  }
}
