#ifndef PARSER_UTIL_HPP_INCLUDED
#define PARSER_UTIL_HPP_INCLUDED

#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/home/phoenix/function/function.hpp>

#include <gmpxx.h>
#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{
  namespace Parser
  {
    struct make_u32string_impl
    {
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

    namespace
    {
      boost::phoenix::function<make_u32string_impl> make_u32string;
    }

    inline mpz_class
    create_mpz(char base, const u32string& s)
    {
      //0-9a-zA-Z
      int actualBase;
      if (base <= '9')
      {
        actualBase = base - '0';
      }
      else if (base <= 'Z')
      {
        #warning john wanted this the other way around but mpz does it this way
        //actualBase = base - 'A' + 26 + 10;
        actualBase = base - 'A' + 10;
      }
      else
      {
        //actualBase = base - 'a' + 10;
        actualBase = base - 'a' + 26 + 10;
      }
      return mpz_class(u32_to_ascii(s), actualBase);
    }

    template <typename Iterator>
    class ExprGrammar;

    template <typename Iterator>
    struct integer_parser
    : public qi::grammar<Iterator, mpz_class(), qi::locals<string_type>>
    {
      integer_parser()
        : integer_parser::base_type(integer)
      {
        namespace ph = boost::phoenix;
        using namespace qi::labels;

        integer =
          (( '0'
          >> (qi::char_('2', '9') | qi::ascii::alpha)
          >> +(qi::ascii::digit | qi::ascii::alpha)[_a += _1])
            [
               _val = ph::bind(&create_mpz, _1, make_u32string(_a))
            ]
          | qi::int_ [qi::_val = qi::_1])
        ;
      }

      qi::rule<Iterator, mpz_class(), qi::locals<string_type>> integer;
    };

    template <typename Iterator>
    class escaped_string_parser : public qi::grammar<Iterator, string_type()>
    {
      public:
      escaped_string_parser()
        : escaped_string_parser::base_type(string)
      {
        using namespace qi::labels;

        string =
          '<'
            >> (
                *(escaped | (qi::standard_wide::char_ - '>'))
                 [
                   _val += _1
                 ]
               )
            >> '>'
        ;

        escaped =
          ('\\' >> qi::standard_wide::char_)
          [_val = _1]
        ;
      }

      private:
      qi::rule<Iterator, string_type()>
        string
      ;

      qi::rule<Iterator, char_type()>
        escaped
      ;
    };

    template <typename Iterator>
    struct ident_parser : public qi::grammar<Iterator, string_type()>
    {
      ident_parser()
      : ident_parser::base_type(ident)
      {
        ident %= qi::ascii::alpha >> *(qi::ascii::alnum | '_')
        ;
      }

      qi::rule<Iterator, string_type()> ident;
    };

    inline void
    setEndDelimiter(Delimiter& d, wchar_t& end)
    {
      end = d.end;
    }

    //inline const
    //string_type& getDelimiterType(const Delimiter& d)
    //{
    //  return d.type;
    //}

    #if 0
    struct handle_expr_error
    {
      template <class ScannerT, class ErrorT>
      Spirit::error_status<>
      operator()(ScannerT const& scan, ErrorT error) const
      {

        //for some reason spirit resets the scanner start position
        //after a retry fails so we need to be past the error before
        //we can keep going
        while (scan.first != error.where)
        {
          ++scan;
        }

        //look for a ;;
        bool found = false;
        bool firstFound = false;
        while (!found && !scan.at_end())
        {
          if (!firstFound)
          {
            if (*scan == ';')
            {
              firstFound = true;
            }
            ++scan;
          }
          else
          {
            if (*scan == ';')
            {
              found = true;
            }
            else
            {
              firstFound = false;
              ++scan;
            }
          }
        }

        //if we get to the end of input without finding a ;; then print
        //the error message
        if (!scan.at_end() && found)
        {
          printErrorMessage(error.where.get_position(), error.descriptor);
        }

        if (!scan.at_end())
        {
          ++scan;
        }

        Spirit::error_status<>::result_t result =
          !found && scan.at_end() ?
          Spirit::error_status<>::fail :
          Spirit::error_status<>::retry;

        return Spirit::error_status<>(result);
      }
    };
    #endif
  }
}

#endif // PARSER_UTIL_HPP_INCLUDED
