#ifndef INTERPRETER_HPP_INCLUDED
#define INTERPRETER_HPP_INCLUDED

#define REMOVE_MAGIC

#include <tl/library.hpp>
#include <tl/types.hpp>
#include <glibmm/convert.h>
#include <tl/evaluator.hpp>
#include <tl/dimtranslator.hpp>
#include <boost/foreach.hpp>
//#include <tl/parser_fwd.hpp>
#include <deque>
#include <boost/assign.hpp>
#include <tl/cache.hpp>
#include <tl/equation.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  /**
   * @brief Interpreter base class.
   *
   * Holds all the data necessary for an interpreter.
   **/
  class Interpreter : public HD
  {
    public:

    Interpreter();
    virtual ~Interpreter();

    typedef std::map<u32string, HD*> IOList;

    void
    addOutput(const IOList& output);

    void
    addInput(const IOList& input);

    void
    addDemand(const u32string& id, const EquationGuard& guard);

    void
    tick();

    /**
     * @brief Loads a library.
     *
     * Initialises the library which will add its types and operations
     * to the type registry.
     **/
    void
    loadLibrary(const ustring_t& name)
    {
      m_lt.loadLibrary(name, *this);
    }

    /**
     * @brief Adds a search path for loading libraries.
     *
     * When a library is loaded the paths added here will be searched
     * for the library in the order they are added.
     **/
    void
    addLibrarySearchPath(const ustring_t& name)
    {
      m_lt.addSearchPath(name);
    }

    void
    registerEquation
    (const u32string& name, const Tuple& validContext, AST::Expr* e);

    #if 0
    HD*
    lookupVariable(const ustring_t& name)
    {
      VariableMap::const_iterator iter = m_variables.find(name);
      return iter != m_variables.end() ? iter->second : 0;
    }
    #endif

    TaggedValue
    operator()(const Tuple& k);

    void
    addExpr(const Tuple& k, HD* h);

    private:
    Libtool m_lt;
    TypeRegistry m_types;
    DimensionTranslator m_dimTranslator;

    size_t m_time;

    IOList m_outputs;
    IOList m_inputs;

    typedef std::map<u32string, EquationGuard> DemandStore;
    DemandStore m_demands;

    //initialises the type indexes
    void
    init_types();

    VariableMap m_variables;

    //adds to id with remaining in the id dimension
    void
    addToVariable
    (
      const u32string& id,
      const u32string& remaining,
      const Tuple& k, HD* e
    );
    //adds to id removing id from the context
    void
    addToVariable(const u32string& id, const Tuple& k, HD* e);
    //does the actual add
    void
    addToVariableActual(const u32string& id, const Tuple& k, HD* e);

    template <typename T>
    HD*
    buildConstantHD(size_t index);

    void
    addDimensionSymbol(const ustring_t& s);

    std::map<u32string, size_t> builtin_name_to_index;

    protected:

    bool m_verbose;

    void cleanupParserObjects();
  };

  TypedValue hash(const TypedValue& dimension, const Tuple& context);
  TypedValue hash(size_t dimension, const Tuple& context);
}

#endif // INTERPRETER_HPP_INCLUDED
