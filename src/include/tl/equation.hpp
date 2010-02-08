#ifndef INTERPRETER_FWD_HPP_INCLUDED
#define INTERPRETER_FWD_HPP_INCLUDED

#include <tl/types.hpp>
#include <boost/shared_ptr.hpp>
#include <tl/exception.hpp>
#include <list>
#include <deque>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{

  namespace AST
  {
    class Expr;
  };

  class Interpreter;

  class InvalidGuard : public Exception
  {
  };

  /**
   * @brief Represents the guard of an equation.
   *
   * The guard is the bit between the @ and the = in
   * eqn @ [dim : E, ...] = expr.
   * The system can impose elements of the guard, it is an error for
   * the user to specify those ones too.
   **/
  class EquationGuard
  {
    public:

    /**
     * @brief Constructs a guard from an AST.
     *
     * Specifies the AST to use for the guard.
     **/
    EquationGuard(HD* g, HD* b)
    : m_guard(g), m_boolean(b)
    {
    }

    /**
     * @brief Creates a guard with no AST.
     *
     * There are no user dimensions in the guard. System imposed
     * dimensions can still be added.
     **/
    EquationGuard()
    : m_guard(0), m_boolean(0)
    {
    }

    EquationGuard(const Tuple& t)
    : m_guard(0), m_boolean(0)
    {
       for (Tuple::const_iterator iter = t.begin();
          iter != t.end();
          ++iter)
       {
          addDimension(iter->first, iter->second);
       }
    }

    /**
     * @brief Determines if there are any dimensions in the guard.
     *
     * @return false if there are no user or system imposed dimensions,
     * true otherwise.
     **/
    operator bool() const
    {
       return m_guard != 0 || m_dimensions.size() != 0 || m_boolean != 0;
    }

    /**
     * @brief Evaluate the guard.
     *
     * Returns a tuple of the dimensions and the evaluated AST,
     * this includes all the system imposed dimensions.
     * @throw InvalidGuard when the user has specified a system imposed
     * dimension.
     **/
    Tuple
    evaluate(const Tuple& context) const
    throw(InvalidGuard);

    /**
     * @brief Adds a system imposed dimension.
     *
     * The system can add dimensions to the guard which the user can't
     * change.
     **/
    void
    addDimension(size_t dim, const TypedValue& v)
    {
       m_dimensions[dim] = v;
    }

    HD*
    boolean() const
    {
       return m_boolean;
    }

    private:
    HD* m_guard;
    HD* m_boolean;
    std::map<size_t, TypedValue> m_dimensions;
  };

  class EquationBase
  {
    public:
    virtual ~EquationBase() {}
    virtual ValueContext evaluate(Interpreter& i, const Tuple& context) = 0;
  };

  //a map from equation name to vector of <valid context, expression>
  typedef std::pair<EquationGuard, EquationBase*> expr_pair_t;
  typedef std::vector<expr_pair_t> expr_pair_v;
  typedef std::map<ustring_t, expr_pair_v> EquationMap;

  typedef boost::shared_ptr<EquationMap> EquationMap_p;

  typedef std::deque<std::pair<EquationGuard, EquationMap_p> > EqnSetList;

  class Variable;
  typedef std::map<u32string, HD*> VariableMap;

  class Equation
  {
    public:
    Equation(const u32string& name, const EquationGuard& valid, HD* h)
    : m_name(name), m_validContext(valid), m_h(h)
    {
    }

    Equation()
    : m_h(0)
    {
    }

    const u32string&
    name() const
    {
       return m_name;
    }

    const EquationGuard&
    validContext() const
    {
       return m_validContext;
    }

    operator bool() const
    {
       return m_h;
    }

    HD*
    equation() const
    {
       return m_h;
    }

    private:
    u32string m_name;
    EquationGuard m_validContext;
    //AST::Expr* m_e;
    //SystemEquation* m_se;
    //EquationBase* m_e;
    HD* m_h;
  };

  //represents all definitions of a variable, is responsible for
  //JIT and best fitting
  class Variable : public HD
  {
    public:

    Variable(const u32string& name, Interpreter& i)
    : m_name(name), m_i(i)
    {}

    TaggedValue operator()(const Tuple& k);

    void
    addExpr(const Tuple& k, HD* h);

    void
    added();

    void
    removed();

    private:

    void
    addExprActual(const Tuple& k, HD* e);

    typedef std::list<Equation> Equations;
    Equations m_equations;
    VariableMap m_variables;

    u32string m_name;
    Interpreter& m_i;
  };
};

#endif // INTERPRETER_FWD_HPP_INCLUDED
