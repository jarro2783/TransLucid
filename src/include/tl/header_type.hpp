#ifndef HEADER_TYPE_HPP_INCLUDED
#define HEADER_TYPE_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/interpreter.hpp>

namespace TransLucid {
   class HeaderType : public TypedValueBase {
      public:

      enum Input {
         DIRECT,
         FILE
      };

      size_t hash() const;

      const Parser::Header& header() const {
         return m_header;
      }

      bool operator==(const HeaderType& rhs) const;

      void parseString(const ustring_t& s, const Tuple& c, Interpreter& i);
      void parseFile(const ustring_t& file, const Tuple& c, Interpreter& i);

      bool operator<(const HeaderType& rhs) const {
         return hash() < rhs.hash();
      }

      private:
      Parser::Header m_header;
   };

   namespace HeaderImp {
      template <HeaderType::Input H>
      struct parser {
      };

      template <>
      struct parser<HeaderType::DIRECT> {
         void operator()(HeaderType& h, const ustring_t& s, const Tuple& c,
            Interpreter& i) const
         {
            h.parseString(s, c, i);
         }
      };

      template <>
      struct parser<HeaderType::FILE> {
         void operator()(HeaderType& h, const ustring_t& s, const Tuple& c,
            Interpreter& i) const
         {
            h.parseFile(s, c, i);
         }
      };

      template <HeaderType::Input H>
      struct name {
      };

      template <>
      struct name<HeaderType::DIRECT> {
         const char* operator()() {
            return "header";
         }
      };

      template <>
      struct name<HeaderType::FILE> {
         const char* operator()() {
            return "headerfile";
         }
      };
   }

   template <HeaderType::Input H>
   class HeaderManager : public TypeManager {

      private:

      public:
      HeaderManager(TypeRegistry& r)
      : TypeManager(r, HeaderImp::name<H>()())
      {}

      TypedValue parse(const ustring_t& s, const Tuple& c, Interpreter& i) const {
         HeaderType h;
         HeaderImp::parser<H>()(h, s, c, i);
         return TypedValue(h, index());
      }

      void printInternal(std::ostream& out, const TypedValue& v, const Tuple& c) const {
      }
   };
};

#endif // HEADER_TYPE_HPP_INCLUDED
