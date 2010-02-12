#ifndef BUILTIN_TYPES_HPP_INCLUDED
#define BUILTIN_TYPES_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/equation.hpp>

namespace TransLucid
{
  class ExprType : public TypedValueBase
  {
    public:

    ExprType(AST::Expr* e)
    : m_expr(e)
    {
    }

    size_t
    hash() const
    {
      boost::hash<AST::Expr*> hasher;
      return hasher(m_expr);
    }

    bool
    operator==(const ExprType& rhs) const
    {
      return m_expr == rhs.m_expr;
    }

    AST::Expr*
    expr() const
    {
       return m_expr;
    }

    bool
    operator<(const ExprType& rhs) const
    {
      return m_expr < rhs.m_expr;
    }

    private:
    AST::Expr* m_expr;
  };

  class Special : public TypedValueBase
  {
    public:
    enum Value
    {
      ERROR,
      ACCESS,
      TYPEERROR,
      DIMENSION,
      UNDEF,
      CONST,
      MULTIDEF,
      LOOP
    };

    Special(const u32string& text)
    : m_v(stringToValue(text))
    {}

    Special(Value v)
    : m_v(v)
    {}

    const Value
    value() const
    {
      return m_v;
    }

    void
    print(std::ostream& os, const Tuple& context) const
    {
      //os << "special<" << m_sv.vtos[m_v] << ">";
    }

    bool
    operator==(const Special& rhs) const
    {
      return m_v == rhs.m_v;
    }

    size_t
    hash() const
    {
      return m_v;
    }

    bool
    operator<(const Special& rhs) const
    {
      return m_v < rhs.m_v;
    }

    private:
    Value m_v;

    struct StringValueInitialiser
    {
      typedef boost::unordered_map<u32string, Value> StringValueMap;
      StringValueMap stov;

      typedef boost::unordered_map<Value, u32string> ValueStringMap;
      ValueStringMap vtos;

      StringValueInitialiser();
    };

    static StringValueInitialiser m_sv;

    public:

    static Value stringToValue(const u32string& s)
    {
      StringValueInitialiser::StringValueMap::const_iterator iter
        = m_sv.stov.find(s);
      if (iter == m_sv.stov.end())
      {
        return ERROR;
      }
      else
      {
        return iter->second;
      }
    }
  };

  #if 0
  class UnevalExpr : public TypedValueBase
  {
    public:
    UnevalExpr(AST::Expr* e, const Tuple& context, Interpreter& i)
    : m_expr(e), m_context(context), m_interpreter(i)
    {
    }

    size_t
    hash() const
    {
      boost::hash<AST::Expr*> hasher;
      return hasher(m_expr);
    }

    AST::Expr*
    expr() const
    {
      return m_expr;
    }

    void
    print(std::ostream& os, const Tuple& context) const
    {
      os << "unevaluated expression";
    }

    bool
    operator==(const UnevalExpr& rhs) const
    {
      return false;
    }

    std::pair<TypedValue, Tuple> evaluate() const;

    bool
    operator<(const UnevalExpr& rhs) const
    {
      return m_expr < rhs.m_expr;
    }

    private:
    //the expression
    AST::Expr* m_expr;
    //the context that the expression was created with
    Tuple m_context;
    //the interpreter that the expression should be evaluated with
    Interpreter& m_interpreter;
  };
  #endif

  class String : public TypedValueBase
  {
    public:
    String(const u32string& s)
    : m_s(s)
    {
    }

    size_t
    hash() const
    {
      boost::hash<u32string> hasher;
      return hasher(m_s);
    }

    static String
    parse(const u32string& text)
    {
      return String(text);
    }

    void
    print(std::ostream& os, const Tuple& c) const
    {
      //os << m_s;
    }

    bool
    operator==(const String& rhs) const
    {
      return m_s == rhs.m_s;
    }

    bool
    operator<(const String& rhs) const
    {
      return m_s < rhs.m_s;
    }

    const u32string&
    value() const
    {
      return m_s;
    }

    private:
    u32string m_s;
  };

  class Boolean : public TypedValueBase
  {
    public:

    Boolean(bool v)
    : m_value(v)
    {
    }

    operator bool() const
    {
      return m_value;
    }

    size_t
    hash() const
    {
      return m_value ? 1 : 0;
    }

    void
    print(std::ostream& os, const Tuple& c) const
    {
      if (m_value)
      {
        os << "true";
      }
      else
      {
        os << "false";
      }
    }

    private:
    bool m_value;
  };

  class Intmp : public TypedValueBase
  {
    public:
    Intmp(const mpz_class& value)
    : m_value(value)
    {
    }

    size_t
    hash() const
    {
      boost::hash<mpz_class> hasher;
      return hasher(m_value);
    }

    void
    print(std::ostream& os, const Tuple& c) const
    {
      os << m_value.get_str();
    }

    bool
    operator==(const Intmp& rhs) const
    {
      return m_value == rhs.m_value;
    }

    const mpz_class&
    value() const
    {
      return m_value;
    }

    bool
    operator<(const Intmp& rhs) const
    {
      return m_value < rhs.m_value;
    }

    private:
    mpz_class m_value;
  };

  class Dimension : public TypedValueBase
  {
    public:
    Dimension(size_t value)
    : m_value(value)
    {}

    size_t
    value() const
    {
      return m_value;
    }

    size_t
    hash() const
    {
      return m_value;
    }

    void
    print(std::ostream& os, const Tuple& c) const
    {
      os << "dimension: '" << m_value << "'";
    }

    bool
    operator==(const Dimension& rhs) const
    {
      return m_value == rhs.m_value;
    }

    bool
    operator<(const Dimension& rhs) const
    {
      return m_value < rhs.m_value;
    }

    private:
    size_t m_value;
  };

  typedef std::set<TypedValue> set_t;

  class Set : public TypedValueBase
  {
    public:
    Set();

    bool
    operator==(const Set& rhs) const;

    void
    print(std::ostream& os, const Tuple& c) const;

    size_t
    hash() const;

    const set_t&
    value() const;
  };

  class ValueCalc : public TypedValueBase
  {
    public:
    size_t
    hash() const
    {
      return 0;
    }

    bool
    operator==(const ValueCalc& rhs) const
    {
      return true;
    }

    bool
    operator<(const ValueCalc& rhs) const
    {
      return false;
    }
  };

  class Char : public TypedValueBase
  {
    public:
    Char(char32_t c)
    : m_c(c)
    {
    }

    char32_t
    value() const
    {
      return m_c;
    }

    size_t
    hash() const
    {
      return m_c;
    }

    void
    print(std::ostream& os, const Tuple& c) const
    {
    }

    bool
    operator==(const Char& rhs) const
    {
      return m_c == rhs.m_c;
    }

    bool
    operator<(const Char& rhs) const
    {
      return m_c < rhs.m_c;
    }

    private:
    char32_t m_c;
  };

  class EquationGuardType : public TypedValueBase
  {
    public:
    EquationGuardType(const EquationGuard& g)
    : m_g(g)
    {}

    const EquationGuard&
    value() const
    {
      return m_g;
    }

    bool
    operator==(const EquationGuardType& rhs) const
    {
      return true;
    }

    bool
    operator<(const EquationGuardType& rhs) const
    {
      return false;
    }

    size_t
    hash() const
    {
      return 0;
    }

    private:
    EquationGuard m_g;
  };

  class PairType : public TypedValueBase
  {
    public:
    PairType(const TypedValue& first, const TypedValue& second)
    : m_first(first), m_second(second)
    {
    }

    const TypedValue&
    first() const
    {
      return m_first;
    }

    const TypedValue&
    second() const
    {
      return m_second;
    }

    size_t
    hash() const
    {
      return 0;
    }

    bool
    operator==(const PairType&) const
    {
      return true;
    }

    bool
    operator<(const PairType&) const
    {
      return false;
    }

    private:
    TypedValue m_first;
    TypedValue m_second;
  };

  class SetBase
  {
    public:
    //is v a member of this
    virtual bool
    is_member(const TypedValue& v) = 0;

    //is s a subset of this
    virtual bool
    is_subset(const SetBase& s) = 0;
  };

  //the general set type
  //all the actual sets will put a derived class in here and then
  //have an is member function.
  class SetType : public TypedValueBase
  {
    public:

    SetType(SetBase* v)
    : m_value(v)
    {}

    bool
    operator==(const SetType& rhs) const
    {
      return m_value == rhs.m_value;
    }

    bool
    operator<(const SetType& rhs) const
    {
      return m_value < rhs.m_value;
    }

    size_t
    hash() const
    {
      return reinterpret_cast<size_t>(m_value);
    }

    private:
    SetBase* m_value;
  };

  class TypeType : public TypedValueBase
  {
    public:

    TypeType(size_t index)
    : m_index(index)
    {}

    bool
    operator==(const TypeType& rhs) const
    {
      return m_index == rhs.m_index;
    }

    bool
    operator<(const TypeType& rhs) const
    {
      return m_index < rhs.m_index;
    }

    size_t
    hash() const
    {
      return m_index;
    }

    size_t
    index() const
    {
      return m_index;
    }

    private:
    size_t m_index;
  };
}

#endif // BUILTIN_TYPES_HPP_INCLUDED
