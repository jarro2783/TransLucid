/* The parser.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

// == The expression parser ==
// Every parse function must leave the iterator untouched if it didn't match,
// and one past the last token of what it matched if it did match
// every function called shall have the iterator at the place to start

/**
 * @file parser-new.cpp
 * The parser.
 */

#include <tl/system.hpp>

#include <functional>
#include <sstream>

#include <tl/charset.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/output.hpp>
#include <tl/parser.hpp>
#include <tl/types/string.hpp>
#include <tl/types_util.hpp>

#define XSTRING(x) STRING(x)
#define STRING(x) #x

namespace TransLucid
{

namespace Parser
{

namespace
{

template <typename Fn, typename... Args>
auto
call_fn(Fn&& f, Args&&... args)
  -> decltype(f(args...))
{
  return f(args...);
}

//bit of a hack, but this one throws away the object in the hope that
//the call works
template <typename Fn, typename T, typename... Args>
auto
call_fn(Fn&& f, T* object, Args&&... args)
  -> decltype(f(args...))
{
  return f(std::forward<Args>(args)...);
}

template <typename Ret, typename T, typename...Args>
Ret
call_fn(Ret (T::*f)(Args...), T* t, Args&&... args)
{
  return (t->*f)(std::forward<Args>(args)...);
}

}

u32string
build_position_message(const Position& pos)
{
  std::ostringstream os;
  os << "at " << pos.file << ":" << pos.line << ":" 
     << pos.character
     << ": ";
  return utf8_to_utf32(os.str());
}

ExpectedToken::ExpectedToken(const Position& pos, 
  size_t token, const u32string& text)
: ParseError(utf32_to_utf8(build_position_message(pos) + 
             U"expected `" + text + U"`"))
, m_token(token)
{
}

ExpectedExpr::ExpectedExpr(const Position& pos, const u32string& text)
: ParseError(utf32_to_utf8(build_position_message(pos) + U"expected " + text))
{
}

template <typename Decls, typename DeclName, typename Obj, typename Fn>
void
add_decl_parser(Decls&& decls, DeclName&& d, Obj&& obj, Fn&& f)
{
  using namespace std::placeholders;
  decls.insert(std::make_pair(d,
    std::bind(std::mem_fn(f), obj, _1, _2, _3)));
}

Parser::Parser(System& system)
: m_system(system)
, m_idents(system.lookupIdentifiers()), m_context(system.getDefaultContext())
{
  //start with top level declarations
  m_which_decl.push(&m_top_decls);

  add_decl_parser(m_where_decls, U"dim", this, &Parser::parse_dim_decl);
  add_decl_parser(m_where_decls, U"var", this, &Parser::parse_var_decl);
  add_decl_parser(m_where_decls, U"fun", this, &Parser::parse_fun_decl);

  add_decl_parser(m_top_decls, U"host", this, &Parser::parse_host_decl);
  add_decl_parser(m_top_decls, U"dim", this, &Parser::parse_dim_decl);
  add_decl_parser(m_top_decls, U"var", this, &Parser::parse_var_decl);
  add_decl_parser(m_top_decls, U"assign", this, &Parser::parse_assign_decl);
  add_decl_parser(m_top_decls, U"in", this, &Parser::parse_in_decl);
  add_decl_parser(m_top_decls, U"hd", this, &Parser::parse_in_decl);
  add_decl_parser(m_top_decls, U"out", this, &Parser::parse_out_decl);
  add_decl_parser(m_top_decls, U"data", this, &Parser::parse_data_decl);
  add_decl_parser(m_top_decls, U"fun", this, &Parser::parse_fun_decl);
  add_decl_parser(m_top_decls, U"op", this, &Parser::parse_op_decl);
}

void
Parser::expect_no_advance(LexerIterator& begin, const LexerIterator& end, 
  const u32string& message,
  size_t token
)
{
  if (begin == end || *begin != token)
  {
    throw ExpectedToken(begin.getPos(), token, message);
  }
}

void
Parser::expect(LexerIterator& begin, const LexerIterator& end, 
  const u32string& message,
  size_t token
)
{
  expect_no_advance(begin, end, message, token);

  ++begin;
}


template <typename Result, typename Fn, typename... Args>
void
Parser::expect(LexerIterator& begin, const LexerIterator& end, 
  Result&& result, const std::u32string& message,
  Fn f,
  Args&&... args
)
{
  LexerIterator current = begin;
  bool success = call_fn(f, this, current, end, std::forward<Result>(result),
    std::forward<Args>(args)...);

  if (!success)
  {
    throw ExpectedExpr(begin.getPos(), message);
  }

  begin = current;
}

inline Tree::UnaryOperator
find_unary_operator
(
  const u32string& symbol,
  const System::IdentifierLookup& idents,
  dimension_index symbolDim,
  Context& k,
  Tree::UnaryType type
)
{
  //lookup ATL_SYMBOL

  ContextPerturber p(k, {{symbolDim, Types::String::create(symbol)}});

  WS* atlWS = idents.lookup(U"ATL_SYMBOL");
  Constant atl = (*atlWS)(k);

  return Tree::UnaryOperator
    {get_constant_pointer<u32string>(atl), symbol, type};
}


/**
 * Finds a binary operator from a string.
 *
 * Tries to match the string @a s with an existing binary operator.
 * If it doesn't exist, then it uses the default error operator and
 * flags the parser that there was a problem.
 * @param s The text of the operator.
 * @param h The parser header.
 * @return The binary operator instance.
 * @todo Implement the error flagging.
 */
inline Tree::BinaryOperator
find_binary_operator
(
  const u32string& symbol, 
  const System::IdentifierLookup& idents,
  dimension_index symbolDim,
  Context& k
)
{
  //lookup ATL_SYMBOL, ASSOC, PREC

  //std::cerr << "looking up symbol " << symbol << std::endl;

  ContextPerturber p(k, {{symbolDim, Types::String::create(symbol)}});
  WS* atlWS = idents.lookup(U"ATL_SYMBOL");
  Constant atl = (*atlWS)(k);

  WS* assocWS = idents.lookup(U"ASSOC");
  Constant assoc = (*assocWS)(k);

  WS* precWS = idents.lookup(U"PREC");
  Constant prec = (*precWS)(k);

  const u32string& assocName = get_constant_pointer<u32string>(assoc);

  Tree::InfixAssoc ia = Tree::ASSOC_LEFT;
  if (assocName == U"LEFT")
  {
    ia = Tree::ASSOC_LEFT;
  }
  else if (assocName == U"RIGHT")
  {
    ia = Tree::ASSOC_RIGHT;
  }
  else if (assocName == U"NON")
  {
    ia = Tree::ASSOC_NON;
  }

  #if 0
  std::cerr << "retrieved op" << std::endl
            << "  symbol: " << symbol << std::endl
            << "  op    : " << get_constant_pointer<u32string>(atl)
            << std::endl
            << "  assoc : " << assocName << std::endl
            << "  prec  : " << get_constant_pointer<mpz_class>(prec)
            << std::endl;
  #endif

  return Tree::BinaryOperator
  {
    ia,
    get_constant_pointer<u32string>(atl),
    symbol,
    get_constant_pointer<mpz_class>(prec)
  };
}

/**
 * Construct a constant with a type.
 * When the type is ustring or uchar, it constructs the actual ustring
 * or uchar. Otherwise it constructs a Tree::LiteralExpr.
 */
inline Tree::Expr
construct_typed_constant(const LexerIterator& token)
{
  auto& val = get<std::pair<u32string, u32string>>(token->getValue());

  const std::u32string& type = val.first;
  const std::u32string& value = val.second;
  if (type == U"ustring") {
    if (!validate_ustring(value)) {
      //TODO error handling
      //throw ParseError(U"Invalid character in ustring");
    }
    return value;
  } else if (type == U"uchar") {
    char32_t v = value[0];
    if (!validate_uchar(v)) {
      //TODO error handling
      //throw ParseError(U"Invalid character");
    }
    return v;
  } else {
    return Tree::LiteralExpr(type, value);
  }
}

bool
Parser::parse_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  return parse_where(begin, end, result);
}

bool
Parser::parse_where(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;
  //binary expression
  Tree::Expr binary_expr;
  bool success = parse_binary_op(current, end, binary_expr);

  if (success)
  {
    Token w = *current;

    if (w == TOKEN_WHERE)
    {
      //TODO parse all of where clause
      m_which_decl.push(&m_where_decls);

      Tree::WhereExpr where;
      try
      {
        ++current;

        bool parsingWhere = true;
        while (parsingWhere)
        {
          Line line;
          if (parse_line(current, end, line))
          {
            //for now just check if it's a var or dim and do the appropriate
            Variable* v = get<Variable>(&line);
            DimensionDecl* d = get<DimensionDecl>(&line);
            if (v != nullptr)
            {
              where.vars.push_back(v->eqn);
            }
            else if (d != nullptr)
            {
              where.dims.push_back(std::make_pair(d->dim, d->initialise));
            }
            else
            {
              //just in case
              throw 
                "internal compiler error at: " __FILE__ ":" XSTRING(__LINE__);
            }
          }
          else
          {
            parsingWhere = false;
          }
        }
      }
      catch(...)
      {
        //in case an expectation throws we should clear up the decl stack
        m_which_decl.pop();
        throw;
      }
      m_which_decl.pop();

      expect(current, end, U"end", TOKEN_END);

      where.e = binary_expr;
      result = where;
    }
    else
    {
      result = binary_expr;
    }
    begin = current;
  }

  return success;
}

bool
Parser::parse_binary_op(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;
  Tree::Expr app;
  bool success = parse_app_expr(current, end, app);
  
  if (success)
  {
    Token t = *current;
    while (t == TOKEN_BINARY_OP)
    {
      //TODO build binary op
      Tree::Expr rhs;
      ++current;
      expect(current, end, rhs, U"app_expr", &Parser::parse_app_expr);

#if 0
      app = Tree::insert_binary_operator
      (
        find_binary_operator
        (
          get<u32string>(t.getValue()),
          m_idents,
          DIM_SYMBOL,
          m_context
        ),
        app, 
        rhs
      );
#endif

      app = Tree::insert_binary_operator
      (
        get<Tree::BinaryOperator>(t.getValue()),
        app, 
        rhs
      );

      t = *current;
    }
    begin = current;
    result = app;
  }

  return success;
}

bool
Parser::parse_app_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;

  Tree::Expr lhs;
  bool success = parse_prefix_expr(current, end, lhs);

  if (success)
  {
    bool parsingApp = true;
    Token next = *current;

    while (parsingApp)
    {
      //this one will modify the lhs and make a binary app expr from it
      if (!parse_token_app(current, end, lhs))
      {
        Tree::Expr rhs;
        bool parsedExpr = parse_prefix_expr(current, end, rhs);

        if (!parsedExpr)
        {
          parsingApp = false;
        }
        else
        {
          //named application
          lhs = Tree::PhiAppExpr(lhs, rhs);
        }
      }
    }
    result = lhs;
    begin = current;
  }

  return success;
}

bool
Parser::parse_token_app(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;
  bool success = true;
  switch (current->getType())
  {
    case TOKEN_AT:
    {
      Tree::Expr rhs;
      ++current;
      expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);
      //build at expr
      result = Tree::AtExpr(result, rhs);
    }
    break;

    case TOKEN_DOT:
    {
      Tree::Expr rhs;
      ++current;
      expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);
      //value application
      result = Tree::LambdaAppExpr(result, rhs);
    }
    break;

    case TOKEN_BANG:
    {
      Tree::Expr rhs;
      ++current;

      if (current->getType() == TOKEN_LPAREN)
      {
        ++current;
        //parse expression list
        LexerIterator listIter = current;
        std::vector<Tree::Expr> exprList;
        Tree::Expr element;

        bool makingList = true;
        while (makingList)
        {
          expect(listIter, end, element, U"expr", &Parser::parse_expr);
          exprList.push_back(std::move(element));

          if (listIter->getType() != TOKEN_COMMA) { makingList = false; }
          else { ++listIter; }
        }

        expect(listIter, end, U")", TOKEN_RPAREN);

        //build a host function with a list of arguments
        result = Tree::BangAppExpr(result, exprList);

        current = listIter;
      }
      else
      {
        expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);
        //host function application with one argument
        result = Tree::BangAppExpr(result, rhs);
      }
    }
    break;

    default:
    //fail
    success = false;
    break;
  }

  if (success) { begin = current; }
  return success;
}

bool
Parser::parse_prefix_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  if (*begin == TOKEN_PREFIX_OP)
  {
    LexerIterator current = begin;
    ++current;
    Tree::Expr rhs;
    expect(current, end, rhs, U"prefix_expr", &Parser::parse_prefix_expr);

    auto op = find_unary_operator(
      get<u32string>(begin->getValue()), 
      m_idents, DIM_SYMBOL, m_context, 
      Tree::UNARY_PREFIX);
    result = Tree::UnaryOpExpr(op, rhs);

    begin = current;
    return true;
  }
  else
  {
    return parse_postfix_expr(begin, end, result);
  }
}

bool
Parser::parse_postfix_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  LexerIterator current = begin;
  Tree::Expr lhs;

  bool success = parse_if_expr(current, end, lhs);

  if (success)
  {
    if (*current == TOKEN_POSTFIX_OP)
    {
      auto op = find_unary_operator(
        get<u32string>(current->getValue()), 
        m_idents, DIM_SYMBOL, m_context, 
        Tree::UNARY_POSTFIX);
      result = Tree::UnaryOpExpr(op, lhs);
      ++current;
    }
    else
    {
      result = lhs;
    }
    begin = current;
  }

  return success;
}

bool
Parser::parse_if_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  if (*begin == TOKEN_IF)
  {
    LexerIterator current = begin;
    ++current;

    Tree::Expr cond;
    expect(current, end, cond, U"expr", &Parser::parse_expr);

    expect(current, end, U"then", TOKEN_THEN);

    Tree::Expr action;
    expect(current, end, action, U"expr", &Parser::parse_expr);
    
    //then some elsifs
    std::vector<std::pair<Tree::Expr, Tree::Expr>> elsifs;
    while (*current == TOKEN_ELSIF)
    {
      ++current;

      Tree::Expr econd;
      expect(current, end, econd, U"expr", &Parser::parse_expr); 

      expect(current, end, U"then", TOKEN_THEN);

      Tree::Expr ethen;
      expect(current, end, ethen, U"expr", &Parser::parse_expr); 

      elsifs.push_back(std::make_pair(econd, ethen));
    }

    expect(current, end, U"else", TOKEN_ELSE);

    Tree::Expr elseexpr;
    expect(current, end, elseexpr, U"expr", &Parser::parse_expr);

    expect(current, end, U"fi", TOKEN_FI);

    result = Tree::IfExpr(cond, action, elsifs, elseexpr);

    begin = current;
    return true;
  }
  else
  {
    return parse_primary_expr(begin, end, result);
  }
}

bool
Parser::parse_primary_expr(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result)
{
  bool success = true;
  switch(begin->getType())
  {
    case TOKEN_INTEGER:
    result = get<mpz_class>(begin->getValue());
    ++begin;
    break;

    case TOKEN_TRUE:
    result = true;
    ++begin;
    break;

    case TOKEN_FALSE:
    result = false;
    ++begin;
    break;

    case TOKEN_DIM_IDENTIFIER:
    result = Tree::DimensionExpr(get<u32string>(begin->getValue()));
    ++begin;
    break;

    case TOKEN_ID:
    result = Tree::IdentExpr(get<u32string>(begin->getValue()));
    ++begin;
    break;

    case TOKEN_CONSTANT:
    result = construct_typed_constant(begin);
    ++begin;
    break;

    case TOKEN_LSQUARE:
    {
      LexerIterator current = begin;
      Tree::Expr tuple;

      expect(current, begin, tuple, U"tuple", 
        &Parser::parse_tuple, SEPARATOR_ARROW);

      result = tuple;
      begin = current;
    }
    break;

    case TOKEN_LPAREN:
    {
      LexerIterator current = begin;
      ++current;
      Tree::Expr e;
      expect(current, end, e, U"expr", &Parser::parse_expr);
      expect(current, end, U")", TOKEN_RPAREN);
      result = e;
      begin = current;
    }
    break;

    case TOKEN_SLASH:
    {
      LexerIterator current = begin;
      ++current;
      parse_function(current, end, result, TOKEN_SLASH);
      begin = current;
    }
    break;

    case TOKEN_DBLSLASH:
    {
      LexerIterator current = begin;
      ++current;
      parse_function(current, end, result, TOKEN_DBLSLASH);
      begin = current;
    }
    break;

    case TOKEN_HASH:
    result = Tree::HashSymbol();
    ++begin;
    break;

    //case TOKEN_UCHAR:
    //result = Tree::

    default:
    success = false;
  }

  return success;
}

void
Parser::parse_function(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result,
  size_t type)
{
  LexerIterator current = begin;

  expect_no_advance(current, end, U"identifier", TOKEN_ID);
  u32string name = get<u32string>(current->getValue());
  ++current;

  expect(current, end, U"->", TOKEN_RARROW);

  Tree::Expr rhs;
  expect(current, end, rhs, U"expr", &Parser::parse_expr);

  if (type == TOKEN_SLASH)
  {
    result = Tree::LambdaExpr(std::move(name), std::move(rhs));
  }
  else if (type == TOKEN_DBLSLASH)
  {
    result = Tree::PhiExpr(std::move(name), std::move(rhs));
  }
  else
  {
    throw "internal compiler error at: " __FILE__ ":" XSTRING(__LINE__);
  }

  //once it has all worked we can move along the iterator
  begin = current;
}

bool 
Parser::parse_tuple(LexerIterator& begin, const LexerIterator& end,
  Tree::Expr& result, TupleSeparator sep)
{
  if (*begin != TOKEN_LSQUARE) { return false; }

  LexerIterator current = begin;
  ++current;

  size_t tok = sep == SEPARATOR_COLON ? TOKEN_COLON : TOKEN_LARROW;

  Tree::TupleExpr::TuplePairs pairs;
  bool parsingTuple = true;
  while (parsingTuple)
  {
    Tree::Expr lhs;
    Tree::Expr rhs;
    expect(current, end, lhs, U"expr", &Parser::parse_expr);
    expect(current, end, U"<-", tok);
    expect(current, end, rhs, U"expr", &Parser::parse_expr);

    pairs.push_back(std::make_pair(lhs, rhs));

    if (*current != TOKEN_COMMA) { parsingTuple = false; }
    else { ++current; }
  }

  expect(current, end, U"]", TOKEN_RSQUARE);

  result = Tree::TupleExpr(pairs);
  begin = current;
  return true;
}

bool
Parser::parse_line(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin == TOKEN_DECLID)
  {
    //get the decl parsers to use
    DeclParsers* decls = m_which_decl.top();

    u32string id = get<u32string>(begin->getValue());

    auto iter = decls->find(id);

    if (iter == decls->end())
    {
      //TODO I don't know how to parse this thing
      std::cerr << "unknown line declaration: " << id << std::endl;
      throw "unknown line declaration";
    }

    LexerIterator current = begin;

    expect(current, end, result, U"line: '" + id + U"'", iter->second);

    expect(current, end, U";;", TOKEN_DBLSEMI);

    begin = current;
    return true;
  }
  else
  {
    return false;
  }
}

bool
Parser::parse_host_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  LexerIterator current = begin;

  if (*current != TOKEN_DECLID || 
      get<u32string>(current->getValue()) != U"host")
  {
    return false;
  }

  ++current;

  expect_no_advance(current, end, U"identifier", TOKEN_ID);

  HostDecl decl;

  decl.identifier = get<u32string>(current->getValue());
  ++current;

  expect(current, end, U"=", TOKEN_EQUALS);

  expect(current, end, decl.expr, U"expr", &Parser::parse_expr);

  result = decl;

  begin = current;

  return true;
}

bool
Parser::parse_dim_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  LexerIterator current = begin;

  if (*current != TOKEN_DECLID || 
      get<u32string>(current->getValue()) != U"dim")
  {
    return false;
  }

  ++current;

  expect_no_advance(current, end, U"identifier", TOKEN_ID);
  //a name and an optional initialiser

  u32string name = get<u32string>(current->getValue());

  ++current;

  Tree::Expr init;
  if (*current == TOKEN_LARROW)
  {
    ++current;
    expect(current, end, init, U"expr", &Parser::parse_expr);
  }

  begin = current;

  result = DimensionDecl(name, init);

  return true;
}

bool
Parser::parse_equation_decl(LexerIterator& begin, const LexerIterator& end,
  Equation& result, size_t separator_symbol, const u32string& separator_text)
{
  LexerIterator current = begin;
  if (*current != TOKEN_ID)
  {
    //this is probably the only way this can fail, after this the beasty that
    //we are parsing should be a var
    return false;
  }

  const u32string& name = get<u32string>(current->getValue());
  ++current;

  Tree::Expr tuple;
  parse_tuple(current, end, tuple, SEPARATOR_COLON);
  
  Tree::Expr boolean;
  if (*current == TOKEN_PIPE)
  {
    ++current;
    expect(current, end, boolean, U"expr", &Parser::parse_expr);
  }

  expect(current, end, separator_text, separator_symbol);

  Tree::Expr expr;
  expect(current, end, expr, U"expr", &Parser::parse_expr);

  result = Equation(name, tuple, boolean, expr);

  begin = current;
  return true;
}

bool
Parser::parse_var_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"var")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  Equation eqn;
  if (parse_equation_decl(current, end, eqn, TOKEN_EQUALS, U"="))
  {
    begin = current;
    result = Variable(std::move(eqn));
    return true;
  }
  return false;
}

bool
Parser::parse_assign_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"assign")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  Equation eqn;
  if (parse_equation_decl(current, end, eqn, TOKEN_ASSIGNTO, U":="))
  {
    begin = current;
    result = Assignment(std::move(eqn));
    return true;
  }
  return false;
}

bool
Parser::parse_hd_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"out")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  Equation eqn;
  bool success = parse_equation_decl(current, end, eqn, TOKEN_EQUALS, U"=");

  if (success)
  {
    result = HDDecl(eqn);
  }

  begin = current;

  return success;
}

bool
Parser::parse_out_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"out")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  Equation eqn;
  bool success = parse_equation_decl(current, end, eqn, TOKEN_EQUALS, U"=");

  if (success)
  {
    result = OutputDecl(eqn);
  }

  begin = current;

  return success;
}

bool
Parser::parse_in_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"in")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  Equation eqn;
  bool success = parse_equation_decl(current, end, eqn, TOKEN_EQUALS, U"=");

  if (success)
  {
    result = InputDecl(eqn);
  }

  begin = current;

  return success;
}

bool
Parser::parse_infix_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID)
  {
    return false;
  }

  const u32string& decl = get<u32string>(begin->getValue());

  //infix[lrnpm] has length 6
  if (decl.size() != 6 || decl.substr(0, 4) == U"infix")
  {
    return false;
  }

  char32_t type = decl[5];
  Tree::InfixAssoc assoc;

  switch(type)
  {
    case 'l':
    assoc = Tree::ASSOC_LEFT;
    break;
    case 'r':
    assoc = Tree::ASSOC_RIGHT;
    break;
    case 'n':
    assoc = Tree::ASSOC_NON;
    break;

    default:
    throw 
      "internal compiler error at: " __FILE__ ":" XSTRING(__LINE__);
    break;
  }

  LexerIterator current = begin;
  ++current;

  u32string symbol;
  expect(current, end, symbol, U"constant", &Parser::is_string_constant);

  u32string name;
  expect(current, end, name, U"constant", &Parser::is_string_constant);

  expect_no_advance(current, end, U"integer", TOKEN_INTEGER);

  mpz_class precedence = get<mpz_class>(current->getValue());
  ++current;

  result = Tree::BinaryOperator(assoc, name, symbol, precedence);

  begin = current;

  return true;
}

bool
Parser::parse_data_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"data")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  expect_no_advance(current, end, U"id", TOKEN_ID);

  DataType data;
  data.name = get<u32string>(current->getValue());
  ++current;

  while (*current == TOKEN_ID)
  {
    data.vars.push_back(get<u32string>(current->getValue()));
    ++current;
  }

  expect(current, end, U"=", TOKEN_EQUALS);

  DataConstructor constructor;
  expect(current, end, constructor, U"data constructor", 
    &Parser::parse_data_constructor);

  data.constructors.push_back(constructor);

  while (*current == TOKEN_PIPE)
  {
    constructor.args.clear();
    ++current;
    expect(current, end, constructor, U"data constructor", 
      &Parser::parse_data_constructor);

    data.constructors.push_back(constructor);
  }

  begin = current;

  result = data;

  return true;
}

bool
Parser::parse_op_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"op")
  {
    return false;
  }

  OpDecl decl;

  LexerIterator current = begin;
  ++current;

  expect_no_advance(current, end, U"operator", TOKEN_OPERATOR);

  decl.optext = get<u32string>(current->getValue());

  ++current;

  expect(current, end, U"=", TOKEN_EQUALS);

  expect(current, end, decl.expr, U"expr", &Parser::parse_expr);

  begin = current;

  result = decl;

  return true;
}

bool
Parser::parse_data_constructor(LexerIterator& begin, const LexerIterator& end,
  DataConstructor& result)
{
  if (*begin != TOKEN_ID)
  {
    return false;
  }

  LexerIterator current = begin;

  result.name = get<u32string>(current->getValue());
  ++current;

  bool parsingConstructor = true;
  while (parsingConstructor)
  {
    if (*current == TOKEN_LPAREN)
    {
      ++current;
      DataConstructor childConst;
      expect(current, end, childConst, U"data constructor",
        &Parser::parse_data_constructor);

      expect(current, end, U")", TOKEN_RPAREN);

      result.args.push_back(childConst);
    }
    else if (*current == TOKEN_ID)
    {
      result.args.push_back(
        DataConstructor{get<u32string>(current->getValue())}
      );
      ++current;
    }
    else
    {
      parsingConstructor = false;
    }
  }

  begin = current;

  return true;
}

bool
Parser::is_string_constant(LexerIterator& begin, const LexerIterator& end,
  u32string& result)
{
  if (*begin != TOKEN_CONSTANT)
  {
    return false;
  }

  auto value = get<std::pair<u32string, u32string>>(begin->getValue());

  if (value.first != U"ustring")
  {
    return false;
  }

  result = value.second;
  ++begin;

  return true;
}

bool
Parser::parse_unary_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  if (*begin != TOKEN_DECLID)
  {
    return false;
  }

  const u32string& decl = get<u32string>(begin->getValue());

  Tree::UnaryType type;
  if (decl == U"prefix")
  {
    type = Tree::UNARY_PREFIX;
  }
  else if (decl == U"postfix")
  {
    type = Tree::UNARY_POSTFIX;
  }
  else
  {
    return false;
  }

  LexerIterator current = begin;

  ++current;

  u32string symbol;
  expect(current, end, symbol, U"string", &Parser::is_string_constant);

  u32string opname;
  expect(current, end, opname, U"string", &Parser::is_string_constant);

  begin = current;

  result = Tree::UnaryOperator(opname, symbol, type);

  return true;
}

bool
Parser::parse_fun_decl(LexerIterator& begin, const LexerIterator& end,
  Line& result)
{
  //id (('.'|' ') id)*

  if (*begin != TOKEN_DECLID || get<u32string>(begin->getValue()) != U"fun")
  {
    return false;
  }

  LexerIterator current = begin;
  ++current;

  expect_no_advance(current, end, U"id", TOKEN_ID);

  FnDecl decl;

  decl.name = get<u32string>(current->getValue());
  ++current;

  bool readingFun = true;

  while (readingFun)
  {
    if (*current == TOKEN_ID)
    {
      decl.args.push_back(std::make_pair(FnDecl::ArgType::CALL_BY_NAME,
        get<u32string>(current->getValue())));
      ++current;
    }
    else if (*current == TOKEN_DOT)
    {
      ++current;
      expect_no_advance(current, end, U"id", TOKEN_ID);
      decl.args.push_back(std::make_pair(FnDecl::ArgType::CALL_BY_VALUE,
        get<u32string>(current->getValue())));
      ++current;
    }
    else
    {
      readingFun = false;
    }
  }

  parse_tuple(current, end, decl.guard, SEPARATOR_COLON);

  if (*current == TOKEN_PIPE)
  {
    ++current;
    expect(current, end, decl.boolean, U"expr", &Parser::parse_expr); 
  }

  expect(current, end, U"=", TOKEN_EQUALS);

  expect(current, end, decl.expr, U"expr", &Parser::parse_expr);

  begin = current;

  result = std::move(decl);

  return true;
}

Token
Parser::nextToken(LexerIterator& begin)
{
  ++begin;
  return *begin;
}

void
LexerIterator::readOne()
{
  Token t = nextToken(*m_next, *m_end, *m_context, m_idents);

  if (t != 0)
  {
    m_pos = m_stream->insert(m_pos, t);
  }
  //otherwise leave the stream at end
}

}
      
}
