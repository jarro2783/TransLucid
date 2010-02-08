#ifndef EXPR_PARSER_HPP_INCLUDED
#define EXPR_PARSER_HPP_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/expr.hpp>
//#include <boost/format.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>

#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/function/function.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>

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

         std::u32string operator()(const u32string& arg1) const
         {
            return arg1;
         }
      };

      function<make_u32string_impl> make_u32string;

      char_type get_end_char(const Delimiter& d) {
         return d.end;
      }

      string_type get_name(const Delimiter& d) {
         return d.type;
      }

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
               >> (-('@' >> at_expr)
               [
                  _val = new_<AST::AtExpr>(_a, _1)
               ]
               | qi::eps [_val = _a]
               )
            ;

            binary_op = prefix_expr [_a = _1]
               >> (*(header.binary_op_symbols
               >> prefix_expr)
                  [
                     _a = ph::bind(&insert_binary_operation, _1, _a, _2)
                  ]
               )
               [
                  _val = _a
               ]
            ;

            prefix_expr
               = (header.prefix_op_symbols >> postfix_expr)
               [
                  _val = new_<AST::UnaryExpr>(_1, _2)
               ]
               | postfix_expr [_val = _1]
               ;

            postfix_expr = hash_expr [_a = _1]
               >> (-(header.postfix_op_symbols
                     [
                        _val = new_<AST::UnaryExpr>(_1, _a)
                     ])
                  | qi::eps
                  [
                     _val = _a
                  ]
                  )
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
               | ident [_a = _1]
               >> (angle_string
                  [
                     _val = new_<AST::ConstantExpr>(make_u32string(_a), make_u32string(_1))
                  ]
                  | qi::eps
                  [
                     _val = new_<AST::IdentExpr>(make_u32string(_a))
                  ]
                  )
               //| self.header.equation_symbols [
               //   push_front(ph::ref(expr_stack), new_<AST::IdentExpr>(
               //      at(ph::ref(self.header.equation_names), arg1))
               //   )
               //]
               //|   constant
               | context_perturb [_val = _1]
               | ('(' >> expr > ')') [_val = _1]
               | header.delimiter_start_symbols
                  [
                     _b = _1,
                     _a = construct<string_type>()
                     //_val = new_<AST::ConstantExpr>(U"a", U"b")
                  ]
                  >> (
                        *(qi::standard_wide::char_ - end_delimiter(ph::bind(&get_end_char, _b)))
                        [_a += _1]
                     )
                     [
                        _val = new_<AST::ConstantExpr>(make_u32string(ph::bind(get_name, _b)), make_u32string(_1))
                     ]
                  >> end_delimiter(ph::bind(get_end_char, _b))
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

            end_delimiter = qi::standard_wide::char_(_r1);

            //constant = self.parsers.constant_parser.top();

            #warning need to fix the modules up here
            //context_perturb = parsers.tuple_parser.top();
         }

         private:

         char_type end;

         qi::rule<Iterator, AST::Expr*()>
            expr,
            if_expr,
            boolean,
            range_expr,
            integer,
            prefix_expr,
            hash_expr,
            context_perturb
         ;

         qi::rule<Iterator, void(char_type)>
            end_delimiter
         ;

         qi::rule<Iterator, AST::Expr*(), qi::locals<AST::Expr*>>
            postfix_expr,
            at_expr,
            binary_op
         ;

         qi::rule<Iterator, AST::Expr*(), qi::locals<string_type, Delimiter>>
            primary_expr
         ;

         integer_parser<Iterator> integer_grammar;
         escaped_string_parser<Iterator> angle_string;
         ident_parser<Iterator> ident;

         #warning error checking
      };

   }

}

#endif // EXPR_PARSER_HPP_INCLUDED
