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

      void reactive(bool r)
      {
        m_reactive = r;
      }

      void run();

      private:
      bool m_verbose;
      bool m_reactive;
      Grammar<Parser::string_type::const_iterator>* m_grammar;
    };
  }
}
