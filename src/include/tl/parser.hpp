#ifndef TL_PARSER_INCLUDED
#define TL_PARSER_INCLUDED

#include <list>
#include <map>
#include <boost/foreach.hpp>
#include <tl/expr.hpp>
#include <deque>
#include <boost/format.hpp>
#include <wchar.h>
#include <boost/shared_array.hpp>
#include <tl/exception.hpp>

//#include <boost/spirit/include/phoenix_core.hpp>

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

namespace TransLucid
{
  namespace Parser
  {
    using namespace boost::phoenix;
    namespace ph = boost::phoenix;

    struct add_dimension_impl
    {
      template <typename Arg1, typename Arg2>
      struct result
      {
        typedef void type;
      };

      template <typename Arg1, typename Arg2>
      void
      operator()(Arg1 arg1, Arg2 arg2) const
      {
        arg1.dimension_symbols.add
          (arg2.c_str(), std::u32string(arg2.begin(), arg2.end()));
      }
    };

    namespace
    {
      function<add_dimension_impl> add_dimension;
    }

    //typedef Spirit::position_iterator<std::string::const_iterator>
    //        iterator_t;

    #if 0
    inline void
    addDimensions
    (
      const std::vector<ustring_t>& dims,
      qi::symbols<>& dimsyms
    )
    {
      size_t i = dims.size();
      BOOST_FOREACH(const ustring_t& s, dims)
      {
        dimsyms.add(s.begin(), s.end(), i);
        ++i;
      }
    }
    #endif

    #if 0
    inline void
    addOpDefinition
    (
      Header& header,
      InfixAssoc assoc,
      const std::u32string& op,
      const std::u32string& symbol,
      AST::Expr* precedence
    )
    {
      size_t pos = header.binary_op_info.size();
      if (symbol.size() == 1)
      {
         header.binary_op_symbols.add(symbol.c_str(), pos);
      }
      std::u32string underscoreSymbol = U"_" + symbol + U"_";
      header.binary_op_symbols.add(underscoreSymbol.c_str(), pos);
      AST::IntegerExpr* p = dynamic_cast<AST::IntegerExpr*>(precedence);
      header.binary_op_info.push_back
        (BinaryOperation(assoc, op, symbol, p->m_value));
    }
    #endif

    inline void
    addDelimiter
    (
      Header& header,
      const string_type& type,
      const string_type& open,
      const string_type& close
    )
    {
      //std::cout << "adding " << open << " " << close << std::endl;
      //if
      //(
         header.delimiter_start_symbols.add
         (
           open.c_str(),
           Delimiter(type, open[0], close[0])
         );
      //)
      //{
      //  throw ParseError("open delimiter '" + ustring_t(open)
      //                                      + "' already defined");
      //}
      //else
      //{
      //}
    }

    template <typename Iterator>
    class HeaderGrammar : public qi::grammar<Iterator, skip, Header()>
    {
      public:

      HeaderGrammar()
      : HeaderGrammar::base_type(headerp)
      {
         using namespace qi::labels;

         assoc_symbols.add
           (L"infixl", ASSOC_LEFT)
           (L"infixr", ASSOC_RIGHT)
           (L"infixn", ASSOC_NON)
           (L"infixp", ASSOC_COMPARISON)
           (L"infixm", ASSOC_VARIABLE)
         ;

         headerp =
           *( headerItem(_a) >> qi::string( ";;" ))
            >> qi::eoi[_val = _a];
         ;

         headerItem =
           (
             qi::lit("dimension")
               > angle_string
                 [
                    add_dimension(_r1, _1)
                 ]
           )
         | (
               assoc_symbols
            >> "ustring"
            >> angle_string
            >> "ustring"
            >> angle_string
            >> integer
           )

         | (
               qi::string("delimiters")
            >> "ustring"
            >> angle_string
            >> "uchar"
            >> angle_string
            >> "uchar"
            >> angle_string
           )
         | (
               qi::string("library")
            >> "ustring"
            >> angle_string
           )
         |    (
                qi::string("prefix")
              | "postfix"
              )
           >> "ustring"
           >> angle_string
           >> "ustring"
           >> angle_string
         ;

         integer = qi::int_;

         //constant = self.parsers.constant_parser.top();

         //BOOST_SPIRIT_DEBUG_RULE(constant);
      }

      private:

      qi::rule<Iterator, skip, Header(), qi::locals<Header>>
         headerp
      ;

      qi::rule<Iterator, skip, void(Header)>
         headerItem
      ;

      qi::rule<Iterator>
         integer
      ;

      //qi::rule<Iterator, string_type()> angle_string;

      escaped_string_parser<Iterator> angle_string;

      qi::symbols<char_type, InfixAssoc> assoc_symbols;
      InfixAssoc currentAssoc;

      //Spirit::assertion<ParseErrorType> expect_dbl_semi;

      //AngleStringGrammar angle_string;
    };

    typedef std::tuple<string_type, AST::Expr*, AST::Expr*, AST::Expr*>
    ParsedEquation;

    template <typename Iterator>
    class EquationGrammar : public qi::grammar<Iterator, ParsedEquation()>
    {
      public:

      EquationGrammar()
      : EquationGrammar::base_type(equation)
      {
        using namespace qi::labels;

        equation =
          (
              ident
           >> guard
           >> boolean
           >> '='
           >> expr
          )
          [
            _val = construct<Equation>(_1, _2, _3, _4)
          ]
        ;

        guard =
          -((qi::lit('@') >> context_perturb[_val = _1]) | qi::eps[_val = 0]);

        boolean = -((qi::lit('|') >> expr[_val = _1]) | qi::eps[_val = 0]);
      }

      template <typename T>
      void
      set_context_perturb(const T& t) {
        context_perturb = t;
      }

      private:

      qi::rule<Iterator, ParsedEquation()>
        equation
      ;

      qi::rule<Iterator, AST::Expr*()>
        guard,
        boolean,
        context_perturb,
        expr
      ;

      ident_parser<Iterator> ident;
    };
  }
}

#endif // TL_PARSER_INCLUDED
