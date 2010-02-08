#include <tl/types.hpp>
#include <boost/foreach.hpp>
//#include <boost/bind.hpp>
#include <tl/range.hpp>
#include <tl/interpreter.hpp>
#include <boost/assign.hpp>
#include <tl/exception.hpp>
#include <tl/header_type.hpp>
#include <tl/footer_type.hpp>

using boost::assign::map_list_of;

namespace TransLucid
{

TypeRegistry::TypeRegistry(Interpreter& i)
: m_nextIndex(1), m_indexError(0), m_interpreter(i)
{
  #if 0
  TypeManager* m = new SpecialManager(*this);
  //makeOpTypeError =
  //  boost::bind(&TypeRegistry::makeOpTypeErrorActual, this, m);
  m_indexSpecial = m->index();
  m = new UnevalManager(*this);
  m_indexUneval = m->index();

  RangeManager* rm = new RangeManager(*this, "_range");
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
  m = new PairManager(*this, "_pair");
  m_indexPair = m->index();
  #endif
  //new HeaderManager<HeaderType::DIRECT>(*this);
}

TypeRegistry::~TypeRegistry()
{
  //doesn't really matter which one I use here to clean up, as long as
  //I only use one of them
  BOOST_FOREACH(IndexTypeMap::value_type& v, m_typeIndexMapping)
  {
     delete v.second;
  }
}

//TypedValue TypeRegistry::makeOpTypeErrorActual(const TypeManager* special)
//{
//  return TypedValue(Special(Special::TYPEERROR), special->index());
//}

Tuple::Tuple()
: m_value(new tuple_t)
{
}

Tuple::Tuple(const tuple_t& tuple)
: m_value(new tuple_t(tuple))
{
}

Tuple
Tuple::insert(size_t key, const TypedValue& value) const
{
  tuple_t t = *m_value;
  t.insert(std::make_pair(key, value));
  return Tuple(t);
}

} //namespace TransLucid
