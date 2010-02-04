#ifndef PARSER_UTIL_HPP_INCLUDED
#define PARSER_UTIL_HPP_INCLUDED

#define PHOENIX_LIMIT 10

#include <boost/spirit/include/classic.hpp>

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
      using namespace boost::phoenix;
      using namespace boost::phoenix::arg_names;
      using namespace boost::phoenix::local_names;

      namespace ph = boost::phoenix;

      template <class Op, int N>
      struct operate_n_imp {

         //bit of a hack because one thing seems to give it one
         //arg and another gives it two, but the arg is
         //completely irrelevant
         template <typename C, typename Arg1 = std::string>
         struct result
         {
             typedef void type;
         };

         template <class C>
         void operator()(C& c) const {
            for (int i = 0; i != N; ++i) {
               m_op(c);
            }
         }

         template <typename RT, typename Env, typename A0>
         static void
         eval(Env const& env, A0& _0)
         {
            Op op;
            for (int i = 0; i != N; ++i) {
               op(_0.eval(env));
            }

            return;
         }

         Op m_op;
      };

      template <int N>
      inline function<operate_n_imp<boost::phoenix::stl::pop_front, N> >
      pop_front_n()
      {
         return function<operate_n_imp<boost::phoenix::stl::pop_front, N> >();
      }

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

      struct integer_parser {
         typedef mpz_class result_t;

         template <typename ScannerT>
         std::ptrdiff_t
         operator()(ScannerT const& scan, result_t& result) const {
            wstring_t s;
            Spirit::rule<ScannerT> integer =
               ( Spirit::lexeme_d
                  [
                     '0'
                     >> (Spirit::range_p('2', '9') | Spirit::alpha_p)
                     >> +(Spirit::digit_p | Spirit::alpha_p)
                  ]
                  | Spirit::int_p )
                  [
                     ph::ref(s) = construct<wstring_t>(arg1, arg2)
                  ]
               ;
            typename Spirit::parser_result<Spirit::rule<ScannerT>, ScannerT>
               ::type p = integer.parse(scan);

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
         }
      };

      struct ident_parser {
         typedef wstring_t result_t;

         template <typename ScannerT>
         std::ptrdiff_t
         operator()(ScannerT const& scan, result_t& result) const {
            wstring_t s;

            typedef
            boost::spirit::classic::action<boost::spirit::classic::contiguous<boost::spirit::classic::sequence<boost::spirit::classic::alpha_parser, boost::spirit::classic::kleene_star<boost::spirit::classic::alternative<boost::spirit::classic::alnum_parser, boost::spirit::classic::chlit<char> > > > >, boost::phoenix::actor<boost::phoenix::composite<boost::phoenix::assign_eval, boost::fusion::vector<boost::phoenix::reference<TransLucid::wstring_t>, boost::phoenix::composite<boost::phoenix::detail::construct_eval<TransLucid::wstring_t>, boost::fusion::vector<boost::phoenix::argument<0>, boost::phoenix::argument<1>, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_> >, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_, boost::fusion::void_> > > >
            ident_t;

            ident_t ident
               = Spirit::lexeme_d
               [
                  (Spirit::alpha_p >> *(Spirit::alnum_p | '_'))
               ]
               [
                  ph::ref(s) = construct<wstring_t>(arg1, arg2)
               ]
               ;

            typename Spirit::parser_result<ident_t, ScannerT>::type p
               = ident.parse(scan);

            if (p.length() > 0) {
               result = s;
            }
            return p.length();
         }
      };

      namespace {
         Spirit::functor_parser<integer_parser> const integer_p;
         Spirit::functor_parser<parse_type_value> const type_value_p = parse_type_value();
         Spirit::functor_parser<ident_parser> identifier_p;
      }

      class AngleStringGrammar : public Spirit::grammar<AngleStringGrammar> {
         public:

         AngleStringGrammar(std::deque<wstring_t>& string_stack)
         : string_stack(string_stack)
         {
         }

         std::deque<wstring_t>& string_stack;

         template <typename ScannerT>
         class definition {
            public:
            definition(const AngleStringGrammar& self)
            : string_stack(self.string_stack)
            {
               angle_string
                  =  Spirit::lexeme_d
                     [
                        '<'
                        >> type_value_p[(push_front(ph::ref(string_stack), arg1))]
                        >> '>'
                     ]
                  ;
            }

            const Spirit::rule<ScannerT>& start() const {
               return angle_string;
            }

            private:
            Spirit::rule<ScannerT> angle_string;

            std::deque<wstring_t>& string_stack;
         };
      };

      inline void setEndDelimiter(Delimiter& d, wchar_t& end) {
         end = d.end;
      }

      inline const wstring_t& getDelimiterType(const Delimiter& d) {
         return d.type;
      }

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
   }

}

#endif // PARSER_UTIL_HPP_INCLUDED
