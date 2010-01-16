#ifndef BUILTIN_TYPES_HPP_INCLUDED
#define BUILTIN_TYPES_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/equation.hpp>

namespace TransLucid {

   class ExprType : public TypedValueBase {
      public:

      ExprType(AST::Expr *e)
      : m_expr(e)
      {
      }

      size_t hash() const {
         boost::hash<AST::Expr*> hasher;
         return hasher(m_expr);
      }

      bool operator==(const ExprType& rhs) const {
         return m_expr == rhs.m_expr;
      }

      AST::Expr *expr() const {
          return m_expr;
      }

      bool operator<(const ExprType& rhs) const {
         return m_expr < rhs.m_expr;
      }

      private:
      AST::Expr *m_expr;
   };

   class Special : public TypedValueBase {
      public:
      enum Value {
         ERROR,
         ACCESS,
         TYPEERROR,
         DIMENSION,
         UNDEF,
         CONST,
         MULTIDEF,
         LOOP
      };

      Special(const ustring_t& text)
      : m_v(stringToValue(text))
      {}

      Special(Value v)
      : m_v(v)
      {}

      const Value value() const {
         return m_v;
      }

      void print(std::ostream& os, const Tuple& context) const {
         os << "special<" << m_sv.vtos[m_v] << ">";
      }

      bool operator==(const Special& rhs) const {
         return m_v == rhs.m_v;
      }

      size_t hash() const {
         return m_v;
      }

      bool operator<(const Special& rhs) const {
         return m_v < rhs.m_v;
      }

      private:
      Value m_v;

      struct StringValueInitialiser {
         typedef boost::unordered_map<ustring_t, Value> StringValueMap;
         StringValueMap stov;

         typedef boost::unordered_map<Value, ustring_t> ValueStringMap;
         ValueStringMap vtos;

         StringValueInitialiser();
      };

      static StringValueInitialiser m_sv;

      public:
      static Value stringToValue(const ustring_t& s) {
         StringValueInitialiser::StringValueMap::const_iterator iter
            = m_sv.stov.find(s);
         if (iter == m_sv.stov.end()) {
            return ERROR;
         } else {
            return iter->second;
         }
      }
   };

   class UnevalExpr : public TypedValueBase {
      public:
      UnevalExpr(AST::Expr* e, const Tuple& context, Interpreter& i)
      : m_expr(e), m_context(context), m_interpreter(i)
      {
      }

      size_t hash() const {
         boost::hash<AST::Expr*> hasher;
         return hasher(m_expr);
      }

      AST::Expr *expr() const {
         return m_expr;
      }

      void print(std::ostream& os, const Tuple& context) const {
         os << "unevaluated expression";
      }

      bool operator==(const UnevalExpr& rhs) const {
         return false;
      }

      std::pair<TypedValue, Tuple> evaluate() const;

      bool operator<(const UnevalExpr& rhs) const {
         return m_expr < rhs.m_expr;
      }

      private:
      //the expression
      AST::Expr *m_expr;
      //the context that the expression was created with
      Tuple m_context;
      //the interpreter that the expression should be evaluated with
      Interpreter& m_interpreter;
   };

   class String : public TypedValueBase {
      public:
      String(const ustring_t& s)
      : m_s(s)
      {
      }

      size_t hash() const {
         boost::hash<ustring_t> hasher;
         return hasher(m_s);
      }

      static String parse(const ustring_t& text) {
         return String(text);
      }

      void print(std::ostream& os, const Tuple& c) const {
         os << m_s;
      }

      bool operator==(const String& rhs) const {
         return m_s == rhs.m_s;
      }

      bool operator<(const String& rhs) const {
         return m_s < rhs.m_s;
      }

      const ustring_t& value() const {
         return m_s;
      }

      private:
      ustring_t m_s;
   };

   class Boolean : public TypedValueBase {
      public:

      Boolean(bool v)
      : m_value(v)
      {
      }

      operator bool() const {
         return m_value;
      }

      size_t hash() const {
         return m_value ? 1 : 0;
      }

      void print(std::ostream& os, const Tuple& c) const {
         if (m_value) {
            os << "true";
         } else {
            os << "false";
         }
      }

      static Boolean parse(const ustring_t& v) {
         if (v == "true") {
            return Boolean(true);
         } else {
            return Boolean(false);
         }
      }

      private:
      bool m_value;
   };

   class Intmp : public TypedValueBase {
      public:
      Intmp(const mpz_class& value)
      : m_value(value)
      {
      }

      static Intmp parse(const ustring_t& text) {
         return Intmp(mpz_class(text.raw()));
      }

      size_t hash() const {
         boost::hash<mpz_class> hasher;
         return hasher(m_value);
      }

      void print(std::ostream& os, const Tuple& c) const {
         os << m_value.get_str();
      }

      bool operator==(const Intmp& rhs) const {
         return m_value == rhs.m_value;
      }

      const mpz_class& value() const {
         return m_value;
      }

      bool operator<(const Intmp& rhs) const {
         return m_value < rhs.m_value;
      }

      private:
      mpz_class m_value;
   };

   class Dimension : public TypedValueBase {
      public:
      Dimension(size_t value)
      : m_value(value)
      {}

      size_t value() const {
         return m_value;
      }

      size_t hash() const {
         return m_value;
      }

      void print(std::ostream& os, const Tuple& c) const {
         os << "dimension: '" << m_value << "'";
      }

      bool operator==(const Dimension& rhs) const {
         return m_value == rhs.m_value;
      }

      bool operator<(const Dimension& rhs) const {
         return m_value < rhs.m_value;
      }

      private:
      size_t m_value;
   };

   typedef std::set<TypedValue> set_t;

   class Set : public TypedValueBase {
      public:
      Set();

      bool operator==(const Set& rhs) const;

      void print(std::ostream& os, const Tuple& c) const;

      size_t hash() const;

      const set_t& value() const;
   };

   class ValueCalc : public TypedValueBase {
      public:
      size_t hash() const {
         return 0;
      }

      bool operator==(const ValueCalc& rhs) const {
         return true;
      }

      bool operator<(const ValueCalc& rhs) const {
         return false;
      }
   };

   class Char : public TypedValueBase {
      public:
      Char(const gunichar c)
      : m_c(c)
      {
      }

      Char(const ustring_t& text) {
         if (text.length() != 1) {
            throw "Invalid char length";
         } else {
            m_c = text.at(0);
         }
      }

      size_t hash() const {
         return m_c;
      }

      static Char parse(const ustring_t& text) {
         return Char(text);
      }

      void print(std::ostream& os, const Tuple& c) const {
         ustring_t s;
         s.push_back(m_c);
         os << s;
      }

      bool operator==(const Char& rhs) const {
         return m_c == rhs.m_c;
      }

      bool operator<(const Char& rhs) const {
         return m_c < rhs.m_c;
      }

      private:
      gunichar m_c;
   };

   class EquationGuardType : public TypedValueBase {
      public:
      EquationGuardType(const EquationGuard& g)
      : m_g(g)
      {}

      const EquationGuard& value() const {
         return m_g;
      }

      private:
      EquationGuard m_g;
   };
}

#endif // BUILTIN_TYPES_HPP_INCLUDED
