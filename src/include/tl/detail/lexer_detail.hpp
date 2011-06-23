/* TransLucid lexer detail.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

/**
 * @file lexer_detail.hpp
 * Lexer implementation details.
 * There are two sets of functions here: build_* and
 * assign_to_attribute_from_iterators.
 * The first construct the different type of tokens at lex time.
 * The second are required to satisfy the compiler because the parser tries
 * to construct semantic values of tokens at match time, but they aren't
 * used because the values are constructed at lex time.
 */

#ifndef TL_LEXER_DETAIL_HPP_INCLUDED
#define TL_LEXER_DETAIL_HPP_INCLUDED

#include <iostream>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <gmpxx.h>

#include <tl/ast.hpp>
#include <tl/charset.hpp>
#include <tl/lexer_util.hpp>
#include <tl/parser_api.hpp>
#include <tl/system.hpp>
#include <tl/utility.hpp>

#define XSTRING(x) STRING(x)
#define STRING(x) #x

namespace TransLucid
{
  namespace Lexer
  {
    namespace lex = boost::spirit::lex;

    namespace detail
    {
      /**
       * Builds a rational value.
       * The lex function class which builds an mpq_class from a string.
       */
      struct build_rational
      {
        /**
         * Builds the rational. The input must be of the form
         * number _ number.
         * @param start The begin iterator.
         * @param end The end iterator.
         * @param matched The pass flags.
         * @param id Something.
         * @param ctx The lexer context.
         */
        template <typename Iterator, typename Idtype, typename Context>
        void
        operator()
        (
          Iterator& start, 
          Iterator& end, 
          lex::pass_flags& matched,
          Idtype& id,
          Context& ctx
        ) const
        {
          try
          {
            //pre: the input string is of the form .+_.+
            mpq_class value;
            Iterator current = start;
            bool negative = false;
            if (*current == L'~')
            {
              negative = true;
              ++current;
            }

            if (*current == L'0')
            {
              //either zero or a nondecrat
              ++current;
              if (*current == L'_')
              {
                //zero, no more needs to be done here
              }
              else
              {
                //nondecrat
                //the current character is the base
                int base = get_numeric_base(*current);
                ++current;

                value = init_mpq(current, end, base);
              }
            }
            else
            {
              //decrat
              value = init_mpq(current, end, 10);
            }
            value.canonicalize();

            if (negative)
            {
              value = -value;
            }

            ctx.set_value(value_wrapper<mpq_class>(value));
          }
          catch (std::invalid_argument& e)
          {
            matched = lex::pass_flags::pass_fail;
          }
        }
      };

      /**
       * Construct a real value. The lex function class which builds an
       * mpf_class from a string.
       */
      struct build_real
      {
        /**
         * Build the float value.
         * @param start The start iterator.
         * @param end The end iterator.
         * @param matched The match flag.
         * @param id Something.
         * @param ctx The lex context.
         */
        template <typename Iterator, typename Idtype, typename Context>
        void
        operator()
        (
          Iterator& start, 
          Iterator& end, 
          lex::pass_flags& matched,
          Idtype& id,
          Context& ctx
        ) const
        {
          try {
            Iterator current = start;
            bool negative = false;
            mpf_class value;

            if (*current == '~')
            {
              negative = true;
              ++current;
            }

            if (*current == '0')
            {
              //either nondec or 0
              ++current;
              if (*current == '.')
              {
                //zero, stop here
              }
              else
              {
                int base = get_numeric_base(*current);
                ++current;
                value = init_mpf(current, end, base);
              }
            }
            else
            {
              //decimal
              value = init_mpf(current, end, 10);
            }

            if (negative)
            {
              value = -value;
            }

            ctx.set_value(value_wrapper<mpf_class>(value));
          }
          catch (std::invalid_argument&)
          {
            matched = lex::pass_flags::pass_fail; 
          }
        }
      };

      /**
       * Build an integer.
       * The lex function class which builds an mpz_class object.
       */
      struct build_integer
      {
        private:
        Parser::Errors& m_errors;

        public:
        /**
         * Construct the integer builder with a parse error tracker.
         */
        build_integer(Parser::Errors& errors)
        : m_errors(errors)
        {
        }

        /**
         * Construct the actual integer. Builds an integer from a string
         * which has been accepted by the lexer as a valid integer. It checks
         * that the number is valid in the base specified. If it is not, an
         * error is recorded.
         * @param first The start iterator.
         * @param last The end iterator.
         * @param matched The match flags.
         * @param id Something.
         * @param ctx The lexer context.
         */
        template <typename Iterator, typename Idtype, typename Context>
        void
        operator()
        (
          Iterator& first, 
          Iterator& last, 
          lex::pass_flags& matched,
          Idtype& id,
          Context& ctx
        ) const
        {
          try
          {
            Iterator next = first;
            ++next;
            mpz_class attr;
            if (next == last && *first == L'0')
            {
              attr = 0;
            }
            else
            {
              //check for negative
              bool negative = false;
              Iterator current = first;
              if (*current == L'~')
              {
                negative = true;
                ++current;
              }

              if (*current == L'0')
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
                  while (current != last)
                  {
                    ++current;
                    ++num;
                  }
                  attr = num;
                }
                else
                {
                  attr = mpz_class(std::string(current, last), base);
                }
              }
              else
              {
                //decint
                //the lexer is guaranteed to have given us digits in the range
                //0-9 now
                attr = mpz_class(std::string(current, last), 10);
              }

              if (negative)
              {
                attr = -attr;
              }
            }
            ctx.set_value(value_wrapper<mpz_class>(attr));
          }
          catch(std::invalid_argument& e)
          {
            //an invalid number was input
            //matched = lex::pass_flags::pass_fail;
            ctx.set_value(value_wrapper<mpz_class>(mpz_class()));
            m_errors.error("invalid integer literal ") << 
              u32string(first, last);
          }
        }
      };
      
      struct build_constant
      {
        /**
         * Create the constant. Reads a constant that is either built with
         * ", `, and/or no type specified.
         * @param first The begin iterator.
         * @param last The end iterator.
         * @param matched The match flags.
         * @param id Something.
         * @param ctx The lexer context.
         */
        template <typename Iterator, typename Idtype, typename Context>
        void
        operator()
        (
          Iterator& first, 
          Iterator& last, 
          lex::pass_flags& matched,
          Idtype& id,
          Context& ctx
        ) const
        {
          //std::cerr << "building string with text " << 
          //  u32string(first, last)
          //  << std::endl;
          u32string type, value;
          bool error = false;

          Iterator current = first;
          while (current != last && *current != '"' && *current != '`')
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
                auto r = build_escaped_characters(current, last);
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
            value = U"==error==";
          }

          ctx.set_value(std::make_pair(type, value));
        }
      };

      struct build_character
      {
        /**
         * Builds a single character.
         * @param first The start iterator.
         * @param last The end iterator.
         * @param matched The match flags.
         * @param id something.
         * @param ctx The lexer context.
         */
        template <typename Iterator, typename Idtype, typename Context>
        void
        operator()
        (
          Iterator& first, 
          Iterator& last, 
          lex::pass_flags& matched,
          Idtype& id,
          Context& ctx
        ) const
        {
          char32_t value = 0;
          bool error = false;

          Iterator current = first;

          //must start with a '
          if (*current == '\'')
          {
            ++current;
            if (*current == '\\')
            {
              //handle escape characters
              auto r = build_escaped_characters(current, last);
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
              if (current != last)
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
            matched = lex::pass_flags::pass_fail;
          }
          else
          {
            ctx.set_value(value);
          }
        }
      };

      //template <typename BinTok>
      struct handle_operator
      {
        private:
        System::IdentifierLookup& m_idents;
        //BinTok& m_binaryOp;
        int m_binaryOp;
        Tuple*& m_context;

        public:
        template <typename BinTok>
        handle_operator(System::IdentifierLookup& idents, BinTok& binary,
          Tuple*& context)
        : m_idents(idents)
        //, m_binaryOp(binary)
        , m_binaryOp(binary.id())
        , m_context(context)
        {
          std::cerr << "The binary op id is " << m_binaryOp << std::endl;
        }

        template <typename Iterator, typename Idtype, typename Context>
        void
        operator()
        (
          Iterator& first, 
          Iterator& last, 
          lex::pass_flags& matched,
          Idtype& id,
          Context& ctx
        ) const
        {
          //look up the operator in the system and set the match information
          //appropriately
          //std::cerr << "The binary op id is " << m_binaryOp.id() << std::endl;
          //id = m_binaryOp.id();
          id = m_binaryOp;

          ctx.set_value(u32string(first, last));
        }
      };
    } //namespace detail
  } //namespace Parser
} //namespace TransLucid

namespace boost { namespace spirit { namespace traits
{
  ///////////////////////////////////////////////////////////////////////////
  // These functions are all here so that parsers don't complain. They would
  // normally convert a pair of iterators to the requested token value type.
  // However, this is being done in the semantic action of the lexer. So
  // these do nothing. The functors that do something are build_rational,
  // build_real and build_integer.
  //////////////////////////////////////////////////////////////////////////
  using TransLucid::Lexer::value_wrapper;
  template <typename Iterator>
  struct assign_to_attribute_from_iterators<value_wrapper<mpz_class>, Iterator>
  //struct assign_to_attribute_from_iterators<mpz_class, Iterator>
  {
    static void
    call
    (
      Iterator const& first, 
      Iterator const& last, 
      value_wrapper<mpz_class>& attr
      //mpz_class& attr
    )
    {
      throw "construct mpz incorrectly called";
    }
  };

  template <typename Iterator>
  struct assign_to_attribute_from_iterators<value_wrapper<mpq_class>, Iterator>
  //struct assign_to_attribute_from_iterators<mpq_class, Iterator>
  {
    static void
    call
    (
      Iterator const& first, 
      Iterator const& last, 
      value_wrapper<mpq_class>& attr
      //mpq_class& attr
    )
    {
      throw "construct mpq incorrectly called";
    }
  };

  template <typename Iterator>
  struct assign_to_attribute_from_iterators<value_wrapper<mpf_class>, Iterator>
  {
    static void
    call
    (
      Iterator const& first, 
      Iterator const& last, value_wrapper<mpf_class>& attr
    )
    {
      throw "construct mpf incorrectly called";
    }
  };

  template <typename Iterator>
  struct assign_to_attribute_from_iterators<
    std::pair<TransLucid::u32string, TransLucid::u32string>, Iterator
  >
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      std::pair<TransLucid::u32string, TransLucid::u32string>& attr
    )
    {
      throw "construct constant pair incorrectly called";
    }
  };
  
  template <typename Iterator>
  struct assign_to_attribute_from_iterators<char32_t, Iterator>
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      char32_t& attr
    )
    {
      //throw "construct character incorrectly called";
      attr = *first;
    }
  };

  template <typename Iterator>
  struct assign_to_attribute_from_iterators
  <
    TransLucid::Tree::InfixAssoc, 
    Iterator
  >
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      TransLucid::Tree::InfixAssoc& attr
    )
    {
      //strings will be of the form infix{l,m,n,p,r}
      TransLucid::u32string s(first, last);
      char32_t c = s.at(s.length()-1);

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
    }
  };

  template <typename Iterator>
  struct assign_to_attribute_from_iterators
  <
    TransLucid::Tree::UnaryType, 
    Iterator
  >
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      TransLucid::Tree::UnaryType& attr
    )
    {
      TransLucid::u32string s(first, last);
      if (s == U"prefix")
      {
        attr = TransLucid::Tree::UNARY_PREFIX;
      }
      else
      {
        attr = TransLucid::Tree::UNARY_POSTFIX;
      }
    }
  };
  
  #if 0
  template <typename Iterator>
  struct assign_to_attribute_from_iterators<TransLucid::u32string, Iterator>
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      TransLucid::u32string& attr
    )
    {
      attr.clear();
      std::cerr << "constructing u32string" << std::endl;
      Iterator current = first;
      while (current != last)
      {
        attr += *current;
        std::cerr << attr << std::endl;
        ++current;
      }
    }
  };
  #endif

  template <>
  struct token_printer_debug<wchar_t>
  {
    template <typename Out>
    static void print(Out& o, wchar_t const& c)
    {
      TransLucid::u32string s(1, c);
      o << s;
    }
  };
}}}

#endif
