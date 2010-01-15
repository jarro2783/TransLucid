#include <tl/types.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <tl/range.hpp>
#include <tl/interpreter.hpp>
#include <boost/assign.hpp>
#include <tl/exception.hpp>
#include <tl/header_type.hpp>
#include <tl/footer_type.hpp>

using boost::assign::map_list_of;

namespace TransLucid {

typedef TemplateTypeManager<Range> RangeManager;
//typedef TemplateTypeManager<Tuple> TupleManager;

namespace {

class ExprManager : public TypeManager {
   public:
   ExprManager(TypeRegistry& r, const ustring_t& name)
   : TypeManager(r, name)
   {
   }

   TypedValue parse(const ustring_t& s,
      const Tuple& context,
      Interpreter& interpreter) const
   {
      AST::Expr *e = interpreter.parseExpr(s);
      return TypedValue(ExprType(e), index());
   }

   void printInternal(std::ostream& os, const TypedValue &v, const Tuple& c) const {
   }
};

class SpecialManager : public TypeManager {
   public:
   SpecialManager(TypeRegistry& r)
   : TypeManager(r, "special")
   {}

   TypedValue parse(const ustring_t& s, const Tuple& context, Interpreter& interpreter) const;

   void printInternal(std::ostream& os, const TypedValue& v, const Tuple& c) const {
      v.value<Special>().print(os, c);
   }
};

TypedValue SpecialManager::parse(const ustring_t& s, const Tuple& context, Interpreter& interpreter) const {
   return TypedValue(Special(s), index());
}

class UnevalManager : public TypeManager {
   public:
   UnevalManager(TypeRegistry& r)
   : TypeManager(r, "_uneval")
   {
   }

   TypedValue parse(const ustring_t& s, const Tuple& context, Interpreter& interpreter) const {
      return TypedValue(Special(Special::CONST), registry().indexSpecial());
   }

   void printInternal(std::ostream& os, const TypedValue& v, const Tuple& c) const {
      os << "_uneval<";
      if (v.index() == registry().indexUneval()) {
         v.value<UnevalExpr>().expr();
      }
       os << ">";
   }
};

//for all the classes that are eagerly evaluated (bool, intmp)
//T must have a function
//TypedValue parse(const ustring_t& text)
template <class T>
class EagerManager : public TypeManager {
   public:
   EagerManager(TypeRegistry& r, const ustring_t& name, bool printName = true)
   : TypeManager(r, name), m_printName(printName)
   {
   }

   TypedValue parse(const ustring_t& s, const Tuple& context, Interpreter& interpreter) const {
      try {
         return TypedValue(T::parse(s), index());
      } catch (...) {
         return TypedValue(Special(Special::CONST), registry().indexSpecial());
      }
   }

   void printInternal(std::ostream& os, const TypedValue& v, const Tuple& c) const {
      if (m_printName) {
         os << name() << "<";
      }
      v.value<T>().print(os, c);
      if (m_printName) {
         os << ">";
      }
   }

   private:
   bool m_printName;
};

template <class T>
class InternalManager : public TypeManager {
   public:
   InternalManager(TypeRegistry& r, const ustring_t& name)
   : TypeManager(r, name)
   {
   }

   TypedValue parse(const ustring_t& s, const Tuple& context, Interpreter& interpreter) const {
      return TypedValue();
   }

   void printInternal(std::ostream& os, const TypedValue& v, const Tuple& c) const {
   }
};

}

typedef EagerManager<Intmp> IntmpManager;
typedef EagerManager<Boolean> BooleanManager;
typedef InternalManager<Dimension> DimensionManager;
typedef EagerManager<String> StringManager;
typedef InternalManager<ValueCalc> CalcManager;
typedef EagerManager<Char> CharManager;
typedef InternalManager<EquationGuardType> EquationGuardManager;

TypeRegistry::TypeRegistry(Interpreter& i)
: m_nextIndex(1), m_indexError(0), m_interpreter(i)
{
   TypeManager *m = new SpecialManager(*this);
   makeOpTypeError = boost::bind(&TypeRegistry::makeOpTypeErrorActual, this, m);
   m_indexSpecial = m->index();
   m = new UnevalManager(*this);
   m_indexUneval = m->index();

   RangeManager *rm = new RangeManager(*this, "_range");
   m_indexRange = rm->index();
   rm->printName(false);

   m = new IntmpManager(*this, "intmp", false);
   m_indexIntmp = m->index();
   m = new BooleanManager(*this, "bool", false);
   m_indexBool = m->index();
   m = new TupleManager(*this, "tuple");
   m_indexTuple = m->index();
   m = new DimensionManager(*this, "_dimension");
   m_indexDimension = m->index();
   m = new ExprManager(*this, "expr");
   m_indexExpr = m->index();
   m = new StringManager(*this, "ustring");
   m_indexString = m->index();
   m = new CalcManager(*this, "_calc");
   m_indexCalc = m->index();
   m = new CharManager(*this, "uchar");
   m_indexChar = m->index();
   m = new EquationGuardManager(*this, "_eguard");
   m_indexGuard = m->index();

   new HeaderManager<HeaderType::DIRECT>(*this);
}

TypeRegistry::~TypeRegistry() {
   //doesn't really matter which one I use here to clean up, as long as
   //I only use one of them
   BOOST_FOREACH(IndexTypeMap::value_type& v, m_typeIndexMapping) {
      delete v.second;
   }
}

void TypeRegistry::registerType(const TypeManager *manager) {
   m_typeIndexMapping.insert(std::make_pair(manager->index(), manager));
   m_typeNameMapping.insert(std::make_pair(manager->name(), manager));
}

TypedValue TypeRegistry::makeOpTypeErrorActual(const TypeManager *special) {
   return TypedValue(Special(Special::TYPEERROR), special->index());
}

Tuple::Tuple()
: m_value(new tuple_t)
{
}

Tuple::Tuple(const tuple_t& tuple)
: m_value(new tuple_t(tuple))
{
}

Tuple Tuple::insert(size_t key, const TypedValue& value) const {
   tuple_t t = *m_value;
   t.insert(std::make_pair(key, value));
   return Tuple(t);
}

std::pair<TypedValue, Tuple> UnevalExpr::evaluate() const {
   return m_interpreter.evaluate(m_expr, m_context);
}

Tuple Tuple::parse(const ustring_t& text, const Tuple& c, Interpreter& i) {
   AST::Expr *e = i.parseExpr(text);
   ValueContext v = i.evaluate(e, c);
   if (v.first.index() != i.typeRegistry().indexTuple()) {
      throw ParseError("tried to construct a tuple with an AST that is not a tuple");
   }
   return v.first.value<Tuple>();
}

void Tuple::print(const Interpreter& i, std::ostream& os, const Tuple& c) const {
   os << "[" << std::endl;
   BOOST_FOREACH(tuple_t::value_type const& v, *m_value) {
      os << v.first << " : ";
      const TypeManager *m = i.typeRegistry().findType(v.second.index());
      m->print(os, v.second, c);
      os << "," << std::endl;
   }
   os << "]" << std::endl;
}

} //namespace TransLucid
