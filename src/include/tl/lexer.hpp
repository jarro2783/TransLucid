/* TransLucid lexer definition.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/home/phoenix/operator.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

#include <tl/charset.hpp>
#include <tl/utility.hpp>
#include <tl/lexer_util.hpp>

namespace TransLucid
{
  namespace Parser
  {
    template <typename T>
    struct value_wrapper;

    #if 0
    template <typename T>
    bool
    operator==(const T& lhs, const value_wrapper<T>& rhs);
    #endif

    template <typename T>
    std::ostream&
    operator<<(std::ostream&, const value_wrapper<T>& rhs);

    template <typename T>
    struct value_wrapper
    {
      public:
      value_wrapper(const T& t)
      : m_t(t)
      {
      }
      
      #if 0
      bool operator==(const T& rhs) const
      {
        return m_t == rhs;
      }
      #endif

      value_wrapper()
      {
      }

      operator T const&() const
      {
        return m_t;
      }

      bool operator==(const value_wrapper<T>& rhs) const
      {
        return m_t == rhs.m_t;
      }

      friend 
      bool 
      operator==(const T& lhs, const value_wrapper<T>& rhs)
      {
        return lhs == rhs.m_t;
      }

      friend
      bool
      operator==(const value_wrapper<T>& lhs, const T& rhs)
      {
        return lhs.m_t == rhs;
      }

      friend
      std::ostream&
      operator<< <T>(std::ostream&, const value_wrapper<T>& rhs);

      private:
      T m_t;
    };

    template <typename T>
    bool
    operator==(const T& lhs, const value_wrapper<T>& rhs)
    {
      return lhs == rhs.m_t;
    }

    template <typename T>
    std::ostream&
    operator<<(std::ostream& os, const value_wrapper<T>& rhs)
    {
      os << rhs.m_t;
      return os;
    }
  }
}

namespace boost { namespace spirit { namespace traits
{
  ///////////////////////////////////////////////////////////////////////////
  // These functions are all here so that parsers don't complain. They would
  // normally convert a pair of iterators to the requested token value type.
  // However, this is being done in the semantic action of the lexer. So
  // these do nothing. The functors that do something are build_rational,
  // build_float and build_integer.
  //////////////////////////////////////////////////////////////////////////
  using TransLucid::Parser::value_wrapper;
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
    std::pair<std::wstring, std::wstring>, Iterator
  >
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      std::pair<std::wstring, std::wstring>& attr
    )
    {
      throw "construct constant pair incorrectly called";
    }
  };
  
  template <typename Iterator>
  struct assign_to_attribute_from_iterators<value_wrapper<wchar_t>, Iterator>
  {
    static void
    call
    (
      Iterator const& first,
      Iterator const& last,
      value_wrapper<wchar_t>& attr
    )
    {
      throw "construct character incorrectly called";
    }
  };
}}}

namespace TransLucid
{
  namespace Parser
  {
    namespace lex = boost::spirit::lex;

    namespace detail
    {
      struct build_rational
      {
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

      struct build_float
      {
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
            std::cerr << "building float from " << std::string(start, end)
              << std::endl;
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

      struct build_integer
      {
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
            mpz_class attr;
            if (last - first == 1 && *first == L'0')
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
                  attr = last - current;
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
            matched = lex::pass_flags::pass_fail;
          }
        }
      };
    }

    struct build_constant
    {
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
        std::wstring type, value;

        Iterator current = first;
        while (current != last && *current != '"' && *current != '`')
        {
          type += *current;
          ++current;
        }

        if (type.empty())
        {
          type = L"ustring";
        }

        ++current;

        if (*current == '"')
        {
          //interpreted
        }
        else
        {
          //raw
          value.reserve(last - current);
          while (*current != '`')
          {
            value += *current;
            ++current;
          }
        }

        std::cerr << "built constant of value " << type << "\"" << value <<
          "\"" << std::endl;
        ctx.set_value(std::make_pair(type, value));
      }
    };

    struct build_character
    {
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
        if (last - first == 3)
        {
          Iterator current = first;
          ++current;
          ctx.set_value(value_wrapper<wchar_t>(*current));
        }
      }
    };

    //for unnamed tokens
    typedef lex::token_def<lex::unused_type, wchar_t> token_def_default;

    template <typename Lexer>
    struct lex_tl_tokens : lex::lexer<Lexer>
    {
      lex_tl_tokens()
      : if_(L"if")
      , fi_(L"fi")
      , where_(L"where")
      , then_(L"then")
      , elsif_(L"elsif")
      , true_(L"true")
      , false_(L"false")
      , spaces(L"[ \\n\\t]")
      {
        using boost::phoenix::ref;
        using lex::_val;
        using lex::_start;
        using lex::_end;
        using lex::_state;
        using lex::_pass;
        namespace ph = boost::phoenix;

        this->self.add_pattern(L"DIGIT", L"[0-9]");
        this->self.add_pattern(L"ADIGIT", L"[0-9A-Za-z]");
        this->self.add_pattern(L"intDEC", L"[1-9]{DIGIT}*");
        this->self.add_pattern(L"intNONDEC", L"0[2-9A-Za-z]{ADIGIT}+");
        this->self.add_pattern(L"intUNARY", L"011+");
        this->self.add_pattern(L"floatDEC", 
          L"{intDEC}\\.{DIGIT}*(\\^~?{ADIGIT}+)?(#{DIGIT}+)?");
        this->self.add_pattern(L"floatNONDEC",
          L"{intNONDEC}\\.{ADIGIT}*(\\^~?{ADIGIT}+)?(#{ADIGIT}+)?");
        this->self.add_pattern(L"ratDEC", L"{intDEC}_{intDEC}");
        this->self.add_pattern(L"ratNONDEC", L"{intNONDEC}_{ADIGIT}+");
        this->self.add_pattern(L"IDENT", L"[A-Za-z][_A-Za-z0-9]*");
        this->self.add_pattern(L"INTERPRETED_STRING", 
          L"\\\"([^\\\"\\\\]|\\.)*\\\"");
        this->self.add_pattern(L"RAW_STRING", L"`[^`]*`");

        identifier = L"{IDENT}";
        constant_raw = L"{IDENT}?{RAW_STRING}";
        constant_interpreted = L"{IDENT}?{INTERPRETED_STRING}";
        integer = L"0|(~?({intDEC}|{intNONDEC}|{intUNARY}))";
        character = L"'(.|\\\\[^\\\\]+)'";

        dblslash = L"\\\\\\\\";
        range = L"\\.\\.";
        arrow = L"->";
        dblsemi = L";;";

        float_val = L"(0\\.0)|~?({floatDEC}|{floatNONDEC})";
        rational = L"(0_1)|(~?)({ratDEC}|{ratNONDEC})";

        this->self =
          spaces[lex::_pass = lex::pass_flags::pass_ignore]
        | if_
        | fi_
        | where_
        | then_
        | elsif_
        | true_
        | false_
        | identifier
        | constant_raw
          [
            build_constant()
          ]
        | constant_interpreted
          [
            build_constant()
          ]
        | character
          [
            build_character()
          ]
        | integer[detail::build_integer()]
        | float_val[detail::build_float()]
        | rational[detail::build_rational()]
        | L':'
        | L'['
        | L']'
        | range
        | L'.'
        | L'='
        | L'&'
        | L'#'
        | L'@'
        | dblslash
        | L'\\'
        | L'('
        | L')'
        | arrow
        | L'|'
        | dblsemi
        ;

        #if 0
        this->self(L"CONSTANT_INTERPRETED") = 
            constant_interpreted
            [
              build_constant(m_constant_type, m_constant_value)
            ]
          | constant_interpreted_escaped
          | constant_interpreted_any
            [
              ph::ref(m_constant_value) +=
                ph::construct<std::wstring>(_start, _end),
              _pass = lex::pass_flags::pass_ignore
            ]
        ;

        this->self(L"CONSTANT_RAW") =
            constant_raw
            [
              build_constant(m_constant_type, m_constant_value)
            ]
          | constant_raw_any
            [
              ph::ref(m_constant_value) += 
                ph::construct<std::wstring>(_start, _end),
              _pass = lex::pass_flags::pass_ignore
            ]
        ;

        constant_raw = L"`";
        constant_raw_any = L".*";

        constant_interpreted = L"\\\"";
        constant_interpreted_escaped = L"\\t|\\a";
        constant_interpreted_any = L".*";
        #endif
      }

      lex::token_def<lex::unused_type, wchar_t> 
        if_, fi_, where_, then_, elsif_, true_, false_
      , spaces
      , arrow, dblsemi, dblslash, range
      ;

      lex::token_def<std::basic_string<wchar_t>, wchar_t>
        identifier
      ;

      lex::token_def<value_wrapper<mpz_class>, wchar_t> 
      //lex::token_def<mpz_class, wchar_t> 
        integer
      ;

      lex::token_def<value_wrapper<mpf_class>, wchar_t>
        float_val
      ;

      lex::token_def<value_wrapper<mpq_class>, wchar_t>
      //lex::token_def<mpq_class, wchar_t>
        rational
      ;

      lex::token_def<std::pair<std::wstring, std::wstring>, wchar_t>
      //these are the two that the parser should match
        constant_raw
      , constant_interpreted
      ;

      lex::token_def<value_wrapper<wchar_t>, wchar_t> character;

      private:
      std::wstring m_constant_type;
      std::wstring m_constant_value;
    };
  }
}
