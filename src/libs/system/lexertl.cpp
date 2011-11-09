/* Lexer using lexertl.
   Copyright (C) 2011 Jarryd Beck

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

#include "lexertl/lookup.hpp"
#include "tl/static_lexer.hpp"
#include "tl/lexertl.hpp"

#include <tl/context.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/lexer_util.hpp>
#include <tl/output.hpp>
#include <tl/system.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#define XSTRING(x) STRING(x)
#define STRING(x) #x

namespace TransLucid
{

namespace Parser
{

Token LexerIterator::m_endToken(Position(), TokenValue(), 0);

namespace
{
  class ValueBuilder
  {
    public:
    ValueBuilder()
    {
      for (int i = 0; i != TOKEN_LAST; ++i)
      {
        m_functions[i] = &buildEmpty;
      }

      m_functions[TOKEN_INTEGER] = &buildInteger;
      m_functions[TOKEN_CONSTANT_RAW] = &buildConstant;
      m_functions[TOKEN_CONSTANT_INTERPRETED] = &buildConstant;
      m_functions[TOKEN_ID] = &buildIdentifier;
      m_functions[TOKEN_UCHAR] = &buildChar;
      m_functions[TOKEN_WHERE] = &buildWhere;
      m_functions[TOKEN_RANGE] = &buildRange;
      m_functions[TOKEN_INFIXBIN] = &buildInfixDecl;
      m_functions[TOKEN_UNARY] = &buildUnaryDecl;
      m_functions[TOKEN_OPERATOR] = &buildOperator;
    }

    template <typename Begin>
    TokenValue
    operator()
    (
      size_t index, 
      Begin&& begin,
      const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      return (*m_functions[index])(std::forward<Begin>(begin), end,
        id, context, idents);
    }

    private:
    typedef TokenValue (*build_func)
    (
      StreamPosIterator begin, 
      const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    );

    build_func m_functions[TOKEN_LAST];

    static TokenValue
    buildRange(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      id = TOKEN_BINARY_OP;
      return u32string(begin, end);
    }

    static TokenValue
    buildEmpty(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      return nil();
    }

    static TokenValue
    buildIdentifier(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      u32string ident(begin, end);
      //lookup ID_TYPE
      WS* lookup = idents.lookup(U"ID_TYPE");

      if (lookup)
      {
        ContextPerturber p(context, {{DIM_ARG0, Types::String::create(ident)}});
        Constant c = (*lookup)(context);
        
        if (c.index() != TYPE_INDEX_USTRING)
        {
          throw "internal compiler error at: " __FILE__ ":" XSTRING(__LINE__);
        }

        const u32string& type = get_constant_pointer<u32string>(c);

        if (type == U"DIM")
        {
          id = TOKEN_DIM_IDENTIFIER;
        }
        else if (type == U"DECLID")
        {
          id = TOKEN_DECLID;
        }
      }

      return ident;
    }

    static TokenValue
    buildChar(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      char32_t value = 0;
      bool error = false;

      StreamPosIterator current = begin;

      //must start with a '
      if (*current == '\'')
      {
        ++current;
        if (*current == '\\')
        {
          //handle escape characters
          auto r = TransLucid::Lexer::build_escaped_characters(current, end);
          if (!r.first || r.second.length() != 1)
          {
            error = true;
          }
          else
          {
            value = r.second[0];
          }
        }
        else
        {
          value = *current;
          ++current;
        }

        //must end with a '
        if (*current != '\'')
        {
          error = true;
        }
        else 
        {
          ++current;
          if (current != end)
          {
            error = true;
          }
        }
      }
      else
      {
        error = true;
      }

      if (error)
      {
        throw "invalid character literal";
      }
      return value;
    }

    static TokenValue
    buildConstant(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      u32string type, value;
      bool error = false;

      StreamPosIterator current = begin;
      while (current != end && *current != '"' && *current != '`')
      {
        type += *current;
        ++current;
      }

      if (type.empty())
      {
        type = U"ustring";
      }

      if (*current == '"')
      {
        ++current;
        //interpreted
        while (*current != '"')
        {
          //start of an escape sequence
          if (*current == '\\')
          {
            //TODO fix this
            auto r = TransLucid::Lexer::build_escaped_characters(current, end);
            if (!r.first)
            {
              error = true;
            }
            else
            {
              value += r.second;
            }
          }
          else
          {
            value += *current;
            ++current;
          }
        }
      }
      else
      {
        //raw
        ++current;
        while (*current != '`')
        {
          value += *current;
          ++current;
        }
      }

      if (error)
      {
        //TODO: set an error flag here
        throw "invalid constant literal";
      }

      return std::make_pair(type, value);
    }

    static TokenValue
    buildInteger(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      try
      {
        StreamPosIterator next = begin;
        ++next;
        mpz_class attr;
        if (next == end && *begin == U'0')
        {
          attr = 0;
        }
        else
        {
          //check for negative
          bool negative = false;
          StreamPosIterator current = begin;
          if (*current == U'~')
          {
            negative = true;
            ++current;
          }

          if (*current == U'0')
          {
            //nondecint
            ++current;
            //this character is the base
            int base = get_numeric_base(*current);

            //we are guaranteed to have at least this character
            ++current;
            if (base == 1)
            {
              //count the number of 1's
              int num = 0;
              while (current != end)
              {
                ++current;
                ++num;
              }
              attr = num;
            }
            else
            {
              attr = mpz_class(std::string(current, end), base);
            }
          }
          else
          {
            //decint
            //the lexer is guaranteed to have given us digits in the range
            //0-9 now
            attr = mpz_class(std::string(current, end), 10);
          }

          if (negative)
          {
            attr = -attr;
          }
        }
        return attr;
      }
      catch(std::invalid_argument& e)
      {
        //TODO: error handling
        throw "invalid integer literal";

#if 0
        //an invalid number was input
        //matched = lex::pass_flags::pass_fail;
        ctx.set_value(value_wrapper<mpz_class>(mpz_class()));
        m_errors.error("invalid integer literal ") << 
          u32string(begin, last);
#endif
      }
    }

    static TokenValue
    buildWhere(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      u32string text(begin, end);

      if (text.size() > 5)
      {
        return text.substr(6, text.size());
      }
      else
      {
        return u32string();
      }
    }

    static TokenValue
    buildOperator(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      //look up the operator in the system and set the match information
      //appropriately

      //need OPTYPE @ [symbol <- u32string(first, last)]
      WS* ws = idents.lookup(U"OPTYPE");

      if (ws == nullptr)
      {
        //TODO fix this
        throw "operator doesn't exist";
      }

      std::u32string text = u32string(begin, end);

      ContextPerturber p(context, 
        {{DIM_SYMBOL, Types::String::create(text)}}
      );

      Constant v = (*ws)(context);

      //the result is either a string or a special, just ignore if not
      //a string
      if (v.index() == TYPE_INDEX_USTRING)
      {
        const u32string& type = get_constant_pointer<u32string>(v);

        if (type == U"BINARY")
        {
          id = TOKEN_BINARY_OP;
        }
        else if (type == U"PREFIX")
        {
          id = TOKEN_PREFIX_OP;
        }
        else
        {
          //postfix is all that is left
          id = TOKEN_POSTFIX_OP;
        }

        return text;
      }
      else
      {
        throw "invalid operator type";
      }
    }
    
    static TokenValue
    buildInfixDecl(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      //strings will be of the form infix{l,m,n,p,r}
      TransLucid::u32string s(begin, end);
      char32_t c = s.at(s.length()-1);
      TransLucid::Tree::InfixAssoc attr;

      switch(c)
      {
        case 'l':
        attr = TransLucid::Tree::ASSOC_LEFT;
        break;

        case 'r':
        attr = TransLucid::Tree::ASSOC_RIGHT;
        break;

        case 'n':
        attr = TransLucid::Tree::ASSOC_NON;
        break;

        case 'p':
        attr = TransLucid::Tree::ASSOC_COMPARISON;
        break;

        case 'm':
        attr = TransLucid::Tree::ASSOC_VARIABLE;
        break;

        default:
        throw "error at: " XSTRING(__LINE__) " in " __FILE__;
      }

      return attr;
    }
    
    static TokenValue
    buildUnaryDecl(StreamPosIterator begin, const StreamPosIterator& end,
      size_t& id,
      Context& context,
      System::IdentifierLookup& idents
    )
    {
      Tree::UnaryType attr;
      TransLucid::u32string s(begin, end);
      if (s == U"prefix")
      {
        attr = TransLucid::Tree::UNARY_PREFIX;
      }
      else
      {
        attr = TransLucid::Tree::UNARY_POSTFIX;
      }
      return attr;
    }

  };

  static ValueBuilder build_value;
}

Token
nextToken
(
  StreamPosIterator& begin, 
  const StreamPosIterator& end,
  Context& context,
  System::IdentifierLookup& idents
)
{
  lexertl::basic_match_results<StreamPosIterator, size_t> results(begin, end);

  translucid_lex(results);

  const StreamPosIterator& match = results.start;

  size_t id = results.id;

  if (id == results.npos())
  {
    //TODO this should be a parse error
    return Token(Position(), TokenValue(), 0);
  }

  //0 is EOF
  if (id < TOKEN_FIRST || id >= TOKEN_LAST)
  {
    std::cerr << match.getLine() << ":" << match.getChar() 
      << ": invalid token: " << id << std::endl;
    //TODO error handling
    throw "Invalid token";
  }

  TokenValue tokVal;

  if (id != 0)
  {
    
    try
    {
      //build up the token
      tokVal = build_value(id, results.start, results.end,
        id, context, idents);
    }
    catch(...)
    {
      //if there is an invalid token then deal with it
      id = 0;
    }
  }

  //set the next StreamPosIterator position
  begin = results.end;

  return Token
  (
    Position
    {
      match.getFile(), 
      match.getLine(), 
      match.getChar(), 
      match.getIterator(), 
      results.end.getIterator()
    },
    tokVal,
    id
  );
}

}

}
