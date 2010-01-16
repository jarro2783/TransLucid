#include <tl/equation.hpp>
#include <tl/interpreter.hpp>
#include <tl/range.hpp>
#include <tl/utility.hpp>

namespace TransLucid {

inline void Variable::addExprActual(const Tuple& k, AST::Expr *e) {
   const EquationGuard *g = 0;
   Tuple::const_iterator giter = k.find(m_i.dimTranslator().lookup("_validguard"));
   if (giter != k.end()) {
      g = &giter->second.value<EquationGuardType const&>().value();
   }

   if (g) {
      m_equations.push_back(Equation(m_name, *g, new ASTEquation(e)));
   } else {
      m_equations.push_back(Equation(m_name, EquationGuard(), new ASTEquation(e)));
   }
}

ValueContext ASTEquation::evaluate(Interpreter& i, const Tuple& context) {
   return i.evaluate(m_e, context);
}

EquationBase::~EquationBase() {
}

bool Variable::tupleApplicable(const Interpreter& i, const Tuple& def, const Tuple& c) const {
   //all of def has to be in c, and the values have to either be
   //equal or within the range
   for (Tuple::const_iterator iter = def.begin(); iter != def.end(); ++iter) {
      Tuple::const_iterator citer = c.find(iter->first);
      if (citer == c.end()) {
         return false;
      } else {
         if (!valueRefines(i, citer->second, iter->second)) {
            return false;
         }
      }
   }

   //if def has nothing it is applicable
   return true;
}


//does value a refine value b
bool Variable::valueRefines(const Interpreter& i, const TypedValue& a, const TypedValue& b) const {
   //if b is a range, a has to be a range and within or equal, or an int and inside
   //otherwise they have to be equal

   const TypeRegistry &types = i.typeRegistry();

   if (b.index() == types.indexRange()) {
      if (a.index() == types.indexRange()) {
         if (!b.value<Range>().within(a.value<Range>())) {
            return false;
         }
      } else if (a.index() == types.indexIntmp()) {
         if (!b.value<Range>().within(a.value<Intmp>())) {
            return false;
         }
      } else {
         return false;
      }
      return true;
   } else {
      return a == b;
   }

   //for now a and b just have to be equal
   return a == b;
}

//does a refine b
bool Variable::tupleRefines(const Interpreter& i, const Tuple& a, const Tuple& b) const {
   //for a to refine b, everything in b must be in a, and for the values that are,
   //they have to be either equal, or their ranges must be more specific
   Tuple::const_iterator it1 = a.begin();
   Tuple::const_iterator it2 = b.begin();
   while (it1 != a.end() && it2 != b.end()) {
      type_index d1 = it1->first;
      type_index d2 = it2->first;

      //extra dimension in b
      if (d2 < d1) {
         return false;
      }

      //extra dimension in a
      if (d1 > d2) {
         ++it1;
         continue;
      }

      if (!valueRefines(i, it1->second, it2->second)) {
         return false;
      }
      ++it1;
      ++it2;
   }

   if (it2 != b.end()) {
      return false;
   }
   return true;
}

bool Variable::booleanTrue(Interpreter& i, const EquationGuard& g, const Tuple& c) const {
   AST::Expr *b = g.boolean();

   if (b) {
      ValueContext v = i.evaluate(g.boolean(), c);

      return v.first.index() == i.typeRegistry().indexBoolean()
      && v.first.value<Boolean>();
   } else {
      return true;
   }
}

TaggedValue Variable::operator()(const Tuple& k) {

   //std::cout << "evaluating variable " << m_name << ", context: " << std::endl;
   //c.print(i, std::cout, c);

   typedef boost::tuple<Tuple, EquationBase*> ApplicableTuple;
   typedef std::list<ApplicableTuple> applicable_list;
   applicable_list applicable;

   //find all the applicable ones

   for (Equations::const_iterator eqn_i = m_equations.begin();
      eqn_i != m_equations.end();
      ++eqn_i)
   {
      if (eqn_i->validContext()) {
         try {
            const EquationGuard& guard = eqn_i->validContext();
            Tuple evalContext = guard.evaluate(m_i, k);
            //std::cout << "guard:" << std::endl;
            //evalContext.print(m_i, std::cout, c);
            if (tupleApplicable(m_i, evalContext, k) && booleanTrue(m_i, guard, k)) {
               applicable.push_back(
                  ApplicableTuple(evalContext, eqn_i->equation()));
            }
         } catch (InvalidGuard& e) {
         }
      } else {
         applicable.push_back(
            ApplicableTuple(
               Tuple(), eqn_i->equation()));
      }
   }

   if (applicable.size() == 0) {
      return TaggedValue(TypedValue(
         Special(Special::UNDEF),
         m_i.typeRegistry().indexSpecial()),k);
   } else if (applicable.size() == 1) {
      return applicable.front().get<1>()->evaluate(m_i, k);
   }

   applicable_list::const_iterator bestIter = applicable.end();

   for (applicable_list::const_iterator iter = applicable.begin();
      iter != applicable.end();
      ++iter)
   {
      if (bestIter == applicable.end()) {
         bestIter = iter;
      } else if (tupleRefines(m_i, iter->get<0>(), bestIter->get<0>())) {
         bestIter = iter;
      } else if (!tupleRefines(m_i, bestIter->get<0>(), iter->get<0>())) {
         bestIter = applicable.end();
      }
   }

   if (bestIter == applicable.end()) {
      return TaggedValue(TypedValue(
         Special(Special::UNDEF),
         m_i.typeRegistry().indexSpecial()), k);
   }

   for (applicable_list::const_iterator iter = applicable.begin();
      iter != applicable.end(); ++iter)
   {
      if (bestIter->get<1>() != iter->get<1>() &&
         !tupleRefines(m_i, bestIter->get<0>(), iter->get<0>()))
      {
         return TaggedValue(TypedValue(
            Special(Special::MULTIDEF),
            m_i.typeRegistry().indexSpecial()), k);
      }
   }

   #warning need to make this the correct value
   //return bestIter->get<1>()->evaluate(m_i, c);

   //return ValueContext(
   //   TypedValue(
   //      Special(Special::ERROR),
   //      m_i.typeRegistry().indexSpecial()),
   //   c);
}

void Variable::addExpr(const Tuple& k, AST::Expr *e) {
   size_t dim_id = m_i.dimTranslator().lookup("id");
   Tuple::const_iterator iter = k.find(dim_id);
   if (iter == k.end()) {
      addExprActual(k, e);
   } else {
      const String *id = iter->second.valuep<String>();
      if (id == 0) {
         return;
      }

      SplitID split(id->value());

      //add the equation, don't add any id dimension if the end is empty
      ustring_t begin = split.first();
      ustring_t end = split.last();

      tuple_t kp = k.tuple();
      if (end.size() != 0) {
         kp[dim_id] = TypedValue(String(end), m_i.typeRegistry().indexString());
      } else {
         kp.erase(dim_id);
      }

      VariableMap::iterator viter = m_variables.find(begin);
      if (viter == m_variables.end()) {
         viter = m_variables.insert(std::make_pair(begin, new Variable(begin, m_i))).first;
      }
      viter->second->addExpr(Tuple(kp), e);
   }
}

}
