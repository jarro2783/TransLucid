#ifndef EXPR_PARSER_HPP_INCLUDED
#define EXPR_PARSER_HPP_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/expr.hpp>
//#include <boost/format.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>

#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/function/function.hpp>
#include <boost/spirit/home/phoenix/container.hpp>

namespace TransLucid {

   namespace Parser {

      using namespace boost::phoenix;
      namespace ph = boost::phoenix;

      struct make_u32string_impl {
         template <typename Arg>
         struct result
         {
            typedef std::u32string type;
         };

         template <typename Arg>
         std::u32string operator()(Arg arg1) const
         {
            return std::u32string(arg1.begin(), arg1.end());
         }
      };

      function<make_u32string_impl> make_u32string;

      template <typename Iterator>
      class ExprGrammar : public qi::grammar<Iterator, AST::Expr*()> {
         Header &header;
         //Parsers& parsers;
         public:

         ExprGrammar(Header& h)
         : ExprGrammar::base_type(expr), header(h)
         {
            using qi::_val;
            using namespace qi::labels;

            expr %= if_expr
            ;

            //stack handling for if
            //when if is seen, push a new list for elsifs
            //when elsif is seen push both expressions onto the list
            //after the whole lot, make an IfExpr with
            //the condition, the expression, the elsif list, and the else
            //expression and pop all
            if_expr =
            (
            qi::lit("if")
               >> if_expr
               >> qi::lit("then") >> if_expr
               >> *("elsif" >> if_expr >>
                  qi::lit("then")
                  >> if_expr)
               >> qi::lit("else") >> if_expr >> qi::lit("fi")
            )
            [
               qi::_val = new_<AST::IfExpr>(_1, _2, _3, _4)
            ]
            | range_expr
            [
               _val = _1
            ]
            ;

            range_expr = at_expr
               ;

               //>> -(".."
               //>> if_expr)
               //;

            //the actions have to be put after the optional with | eps
            at_expr = binary_op [_a = _1]
               >> -('@' >> at_expr)
               [
                  _val = new_<AST::AtExpr>(_a, _1)
               ]
               | qi::eps [_val = _a]
            ;

            binary_op = prefix_expr [_val = _1]
            #warning work out how to do binary ops
            #if 0
               >> *(header.binary_op_symbols
               >> prefix_expr
               )
            #endif
            ;

            prefix_expr
               = //(header.prefix_op_symbols >> postfix_expr)
               postfix_expr [_val = _1]
               ;

            postfix_expr = hash_expr [_val = _1]
               //>> -header.postfix_op_symbols
               ;

            hash_expr = ('#' > hash_expr [_val = new_<AST::HashExpr>(_1)])
            | primary_expr [_val = _1]
            //| primary_expr
            ;

            primary_expr
               = integer [_val = _1]
               |  boolean [_val = _1]
               | header.dimension_symbols
               [
                  //_val = new_<AST::DimensionExpr>(
                  //   at(ph::ref(header.dimension_names), _1))
                  _val = new_<AST::DimensionExpr>(make_u32string(_1))
               ]
               #if 0
               #warning make this a utility parser
               | ((qi::alpha | "_") >> *(qi::alnum | "_"))
               #warning make a utility parser which does backslash escaping
               >> (angle_string | qi::eps )
               //| self.header.equation_symbols [
               //   push_front(ph::ref(expr_stack), new_<AST::IdentExpr>(
               //      at(ph::ref(self.header.equation_names), arg1))
               //   )
               //]
               //|   constant
               |   context_perturb
               |   ('(' >> expr > ')') [_val = _1]
               #if 0
               |   (Spirit::confix_p( self.header.delimiter_start_symbols
               [
                  push_front(ph::ref(string_stack),
                     bind(&getDelimiterType,
                        at(ph::ref(self.header.delimiter_info), arg1)
                     )
                  ),
                  bind(&setEndDelimiter, at(ph::ref(self.header.delimiter_info), arg1),
                     ph::ref(endDelimiter))
               ]
               , (*Spirit::anychar_p)
               [
                  push_front(ph::ref(string_stack), construct<wstring_t>(arg1, arg2))
               ]
               ,  Spirit::ch_p(boost::ref(endDelimiter)) )
               )
               [
                  push_front(ph::ref(expr_stack), new_<AST::ConstantExpr>(
                     at(ph::ref(string_stack), 1),
                     at(ph::ref(string_stack), 0)
                     )),
                  pop_front_n<2>()(ph::ref(string_stack))
               ]
               #endif
               #warning work out how to do delimiters
               #endif
               ;

            boolean = (qi::ascii::string("true") | qi::ascii::string("false"))
            [
               _val = new_<AST::BooleanExpr>(make_u32string(_1))
            ]
            ;

            integer = integer_grammar
            [
               _val = new_<AST::IntegerExpr>(_1)
            ]
            ;

            //constant = self.parsers.constant_parser.top();

            #warning need to fix the modules up here
            //context_perturb = parsers.tuple_parser.top();

            angle_string =
               (qi::ascii::char_('<') >> *(qi::ascii::char_ - '>') >> qi::ascii::char_('>'))
               [
                  _val = _1
               ]
            ;

            #if 0
            BOOST_SPIRIT_DEBUG_NODE(integer);
            BOOST_SPIRIT_DEBUG_NODE(boolean);
            //BOOST_SPIRIT_DEBUG_NODE(constant);
            BOOST_SPIRIT_DEBUG_NODE(primary_expr);
            BOOST_SPIRIT_DEBUG_NODE(hash_expr);
            BOOST_SPIRIT_DEBUG_NODE(postfix_expr);
            BOOST_SPIRIT_DEBUG_NODE(prefix_expr);
            #endif
         }

         private:
         qi::rule<Iterator, AST::Expr*()> expr,
         if_expr,
         binary_op,
         boolean,
         range_expr,
         integer,
         postfix_expr,
         prefix_expr,
         hash_expr,
         primary_expr
         ;

         qi::rule<Iterator, AST::Expr*(), qi::locals<AST::Expr*>>
            at_expr
         ;

         qi::rule<Iterator>
            //constant,
            context_perturb
         ;

         qi::rule<Iterator, string_type>
            angle_string
         ;

         integer_parser<Iterator> integer_grammar;

         //std::deque<size_t> op_stack;
         //std::deque<AST::Expr*>& expr_stack;
         //std::deque<wstring_t>& string_stack;
         //std::deque<std::list<AST::Expr*> > list_stack;

         //errors
         #warning work out how to do error checking
         #if 0
         Spirit::assertion<ParseErrorType>
         expect_fi,
         expect_else,
         expect_colon,
         expect_primary,
         expect_close_paren,
         expect_then
         ;
         #endif

         wchar_t endDelimiter;

         //AngleStringGrammar angle_string;
      };

   }

}

#endif // EXPR_PARSER_HPP_INCLUDED
