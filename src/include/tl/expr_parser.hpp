#ifndef EXPR_PARSER_HPP_INCLUDED
#define EXPR_PARSER_HPP_INCLUDED

#include <tl/parser_util.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/expr.hpp>
#include <tl/utility.hpp>
#include <tl/builtin_types.hpp>
//#include <boost/format.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>

#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>

namespace TransLucid
{
  namespace Parser
  {
    using namespace boost::phoenix;
    namespace ph = boost::phoenix;

    inline char_type
    get_end_char(const Delimiter& d)
    {
      return d.end;
    }

    inline u32string
    get_name(const Delimiter& d)
    {
      return d.type;
    }

    inline AST::Expr*
    construct_delimited_constant(Delimiter& d, const u32string& v)
    {
      if (d.type == U"ustring") {
        return new AST::StringExpr(v);
      } else if (d.type == U"uchar") {
        return new AST::UcharExpr(v[0]);
      } else {
        return new AST::ConstantExpr(d.type, v);
      }
    }

    template <typename Iterator>
    class ExprGrammar
    : public qi::grammar<Iterator, AST::Expr*(), SkipGrammar<Iterator>>
    {
      public:

      ExprGrammar(Header& h)
      : ExprGrammar::base_type(expr), header(h)
      {
        using qi::_val;
        using namespace qi::labels;

        specials.add
          (L"sperror", Special::ERROR)
          (L"spaccess", Special::ACCESS)
          (L"sptype", Special::TYPEERROR)
          (L"spdim", Special::DIMENSION)
          (L"spundef", Special::UNDEF)
          (L"const", Special::CONST)
          (L"loop", Special::LOOP)
        ;

        #if 0
        {Special::ERROR, "error"},
        {Special::ACCESS, "access"},
        {Special::TYPEERROR, "type"},
        {Special::DIMENSION, "dim"},
        {Special::UNDEF, "undef"},
        {Special::CONST, "const"},
        {Special::LOOP, "loop"}
        #endif

        expr = if_expr [_val = _1]
        ;

        if_expr =
          (
              qi::lit("if")
           >> if_expr
           >> qi::lit("then")
           >> if_expr
           >> *(
                    "elsif"
                 >> if_expr
                 >> qi::lit("then")
                 >> if_expr
               )
           >> qi::lit("else")
           >> if_expr
           >> qi::lit("fi")
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
        at_expr =
           binary_op [_a = _1]
        >> (
             ('@' >> at_expr)
             [
               _val = new_<AST::AtExpr>(_a, _1)
             ]
           | qi::eps [_val = _a]
           )
        ;

        binary_op =
           prefix_expr [_a = _1]
        >> (
             *(   header.binary_op_symbols
               >> prefix_expr
              )
              [
                _a = ph::bind(&AST::insert_binary_operation, _1, _a, _2)
              ]
           )
           [
             _val = _a
           ]
        ;

        prefix_expr =
          ( header.prefix_op_symbols >> postfix_expr)
          [
            _val = new_<AST::UnaryExpr>(_1, _2)
          ]
        | postfix_expr [_val = _1]
        ;

        postfix_expr =
           hash_expr [_a = _1]
        >> (
             ( header.postfix_op_symbols
               [
                 _val = new_<AST::UnaryExpr>(_1, _a)
               ]
             )
           | qi::eps
             [
              _val = _a
             ]
           )
        ;

        hash_expr =
          ( '#' > hash_expr [_val = new_<AST::HashExpr>(_1)])
          | primary_expr [_val = _1]
        ;

        primary_expr =
          integer [_val = _1]
        | boolean [_val = _1]
        | header.dimension_symbols
          [
            _val = new_<AST::DimensionExpr>(make_u32string(_1))
          ]
        | specials [_val = new_<AST::SpecialExpr>(_1)]
        | ident [_a = _1]
          >> ( angle_string
               [
                 _val = new_<AST::ConstantExpr>(make_u32string(_a),
                                                make_u32string(_1))
               ]
             | qi::eps
               [
                _val = new_<AST::IdentExpr>(make_u32string(_a))
               ]
             )
        | context_perturb [_val = _1]
        | ('(' >> expr > ')') [_val = _1]
        | (   header.delimiter_start_symbols
              [
                _b = _1,
                _a = construct<string_type>()
              ]
           >> (
               qi::lexeme
               [
                 *(qi::standard_wide::char_ -
                   end_delimiter(ph::bind(&get_end_char, _b)))
                  [
                    _a += _1
                  ]
               ]
              )
            > end_delimiter(ph::bind(&get_end_char, _b))
          )
          [
            _val = ph::bind(&construct_delimited_constant, _b,
                            make_u32string(_a))

            //      new_<AST::ConstantExpr>
            //         (make_u32string(ph::bind(&get_name, _b)),
            //          make_u32string(_a))
          ]
        ;

        boolean =
          ( qi::ascii::string("true") | qi::ascii::string("false"))
          [
            _val = new_<AST::BooleanExpr>(make_u32string(_1))
          ]
        ;

        integer =
          integer_grammar
          [
            _val = new_<AST::IntegerExpr>(_1)
          ]
        ;

        end_delimiter = qi::standard_wide::char_(_r1);

        BOOST_SPIRIT_DEBUG_NODE(if_expr);
        BOOST_SPIRIT_DEBUG_NODE(expr);
        BOOST_SPIRIT_DEBUG_NODE(boolean);
        BOOST_SPIRIT_DEBUG_NODE(range_expr);
        BOOST_SPIRIT_DEBUG_NODE(integer);
        BOOST_SPIRIT_DEBUG_NODE(prefix_expr);
        BOOST_SPIRIT_DEBUG_NODE(hash_expr);
        BOOST_SPIRIT_DEBUG_NODE(context_perturb);
        BOOST_SPIRIT_DEBUG_NODE(end_delimiter);
        BOOST_SPIRIT_DEBUG_NODE(postfix_expr);
        BOOST_SPIRIT_DEBUG_NODE(at_expr);
        BOOST_SPIRIT_DEBUG_NODE(binary_op);
        //BOOST_SPIRIT_DEBUG_NODE(primary_expr);

        qi::on_error<qi::fail>
        (
          expr,
          std::cerr
          << val("Error! Expecting ")
          << _4
          << val(" here: \"")
          << construct<std::string>(_3, _2)
          << val("\"")
          << std::endl
        );
      }

      template <typename T>
      void
      set_context_perturb(const T& t)
      {
        context_perturb = t;
      }

      private:

      char_type end;

      qi::rule<Iterator, AST::Expr*(), SkipGrammar<Iterator>>
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

      qi::rule<Iterator, AST::Expr*(), qi::locals<AST::Expr*>,
               SkipGrammar<Iterator>>
        postfix_expr,
        at_expr,
        binary_op
      ;

      qi::rule<Iterator, AST::Expr*(), qi::locals<string_type, Delimiter>,
               SkipGrammar<Iterator>>
        primary_expr
      ;

      integer_parser<Iterator> integer_grammar;
      escaped_string_parser<Iterator> angle_string;
      ident_parser<Iterator> ident;

      Header &header;

      qi::symbols<char_type, Special::Value> specials;

      #warning error checking
    };

    extern template class ExprGrammar<string_type::const_iterator>;
  }
}

#endif // EXPR_PARSER_HPP_INCLUDED
