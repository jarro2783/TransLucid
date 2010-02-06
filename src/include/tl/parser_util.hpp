#ifndef PARSER_UTIL_HPP_INCLUDED
#define PARSER_UTIL_HPP_INCLUDED

#if 0
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/home/phoenix/object/new.hpp>
#include <boost/spirit/home/phoenix/function/function.hpp>
#include <boost/spirit/home/phoenix/statement/for.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/operator/self.hpp>
#include <boost/spirit/home/phoenix/operator/io.hpp>
#include <boost/spirit/home/phoenix/statement/sequence.hpp>
#include <boost/spirit/home/phoenix/scope/local_variable.hpp>
#include <boost/spirit/home/phoenix/scope/let.hpp>
#include <boost/spirit/home/phoenix/core/argument.hpp>
#include <boost/spirit/home/phoenix/operator/arithmetic.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function.hpp>
#include <boost/spirit/home/phoenix/bind/bind_function_object.hpp>
#include <boost/spirit/home/phoenix/object/delete.hpp>
#include <boost/spirit/home/phoenix/algorithm.hpp>
#endif

#include <boost/spirit/include/phoenix_operator.hpp>

#include <gmpxx.h>
#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>

namespace Glib {
   template <class In>
   struct ustring::SequenceToString<In, wchar_t> : public std::string
   {
      SequenceToString(In begin, In end)
      {
         char c[6];
         for (; begin != end; ++begin) {
            const std::string::size_type len = g_unichar_to_utf8(*begin, c);
            this->append(c, len);
         }
      }
   };
}

namespace TransLucid {

   namespace Parser {

      #if 0
      struct parse_type_value {
         typedef wstring_t result_t;

         template <typename ScannerT>
         std::ptrdiff_t
         operator()(ScannerT const& scan, result_t& result) const {
            if (scan.at_end()) {
               return -1;
            }

            result.clear();

            int angleCount = 0;

            std::ptrdiff_t len = 0;
            wchar_t c;
            while (!scan.at_end() && !((c = *scan) == '>' && angleCount == 0)) {
               if (c == '<') {
                  ++angleCount;
               } else if (c == '>') {
                  --angleCount;
               } else if (c == '\\') {
                  ++scan;
                  c = *scan;
                  if (!scan.at_end()) {
                     ++len;
                  } else {
                     result.append(1, L'\\');
                     continue;
                  }
               }

               result.append(1, c);
               ++scan;
               ++len;
            }

            return len;
         }
      };
      #endif

      template <typename Iterator>
      struct integer_parser : public qi::grammar<Iterator, mpz_class()>
      {
         integer_parser()
         : integer_parser::base_type(integer)
         {
            integer =
               ( '0'
               >> (qi::char_('2', '9') | qi::ascii::alpha)
               >> +(qi::ascii::digit | qi::ascii::alpha)
               | qi::int_ [qi::_val = qi::_1])
               ;

            //bool success = qi::parse(first, last, integer);

            #warning work out how to do this
            #if 0
            if (p.length() > 0) {
               try {
                  wstring_t::const_iterator iter = s.begin();
                  if (s == L"0") {
                     result = mpz_class();
                  } else if (*iter == '0') {
                     ++iter;
                     int baseChar = *iter;
                     int base;

                     if (baseChar >= '2' && baseChar <= '9') {
                        base = baseChar - '0';
                     } else if (baseChar >= 'a' && baseChar <= 'z') {
                        base = baseChar - 'a' + 10;
                     } else if (baseChar >= 'A' && baseChar <= 'Z') {
                        base = baseChar - 'A' + 36;
                     } else {
                        return -1;
                     }

                     ++iter;

                     wstring_t::const_iterator end = s.end();
                     result = mpz_class(ustring_t(wstring_t(iter, end)).raw(), base);

                  } else {
                     result = mpz_class(ustring_t(s).raw(), 10);
                  }
                  return p.length();
               } catch (...) {
                  return -1;
               }
            }
            return -1;
            #endif
         }

         qi::rule<Iterator, mpz_class()> integer;
      };

      struct ident_parser {
         typedef std::u32string result_t;

         template <typename Iterator, typename Context
         , typename Skipper, typename Attribute>
         bool parse(Iterator& first, Iterator const& last
         , Context& context, Skipper const& skipper
         , Attribute& attr) const
         {
            bool r = parse(first, last,
               (qi::ascii::alpha >> *(qi::ascii::alnum | '_'))
            );

            #warning work out how to return result
            return r;
         }
      };

      namespace {
         #if 0
         Spirit::functor_parser<integer_parser> const integer_p;
         Spirit::functor_parser<parse_type_value> const type_value_p = parse_type_value();
         Spirit::functor_parser<ident_parser> identifier_p;
         #endif
      }

      inline void setEndDelimiter(Delimiter& d, wchar_t& end) {
         end = d.end;
      }

      inline const string_type& getDelimiterType(const Delimiter& d) {
         return d.type;
      }

      #if 0
      struct handle_expr_error {
         template <class ScannerT, class ErrorT>
         Spirit::error_status<>
         operator()(ScannerT const& scan, ErrorT error) const {

            //for some reason spirit resets the scanner start position
            //after a retry fails so we need to be past the error before
            //we can keep going
            while (scan.first != error.where) {
               ++scan;
            }

            //look for a ;;
            bool found = false;
            bool firstFound = false;
            while (!found && !scan.at_end()) {
               if (!firstFound) {
                  if (*scan == ';') {
                     firstFound = true;
                  }
                  ++scan;
               } else {
                  if (*scan == ';') {
                     found = true;
                  } else {
                     firstFound = false;
                     ++scan;
                  }
               }
            }

            //if we get to the end of input without finding a ;; then print
            //the error message
            if (!scan.at_end() && found) {
               printErrorMessage(error.where.get_position(), error.descriptor);
            }

            if (!scan.at_end()) {
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
