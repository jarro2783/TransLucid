#include <tl/equation.hpp>
#include <tl/interpreter.hpp>
#include <tl/range.hpp>

namespace TransLucid {

ValueContext ASTEquation::evaluate(Interpreter& i, const Tuple& context) {
   return i.evaluate(m_e, context);
}

EquationBase::~EquationBase() {
}

void Variable::addSet(EqnSetList::const_iterator guard,
   EquationMap::const_iterator set)
{
   if (m_e.find(guard) == m_e.end()) {
      m_e.insert(std::make_pair(guard, set));
   }
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
   for (Equations::const_iterator set_i = m_e.begin();
      set_i != m_e.end();
      ++set_i)
   {
      const EquationGuard& guard = set_i->first->first;

      bool setValid = true;
      if (guard) {
         try {
            setValid = tupleApplicable(m_i, guard.evaluate(m_i, k), k);
         } catch (InvalidGuard& e) {
            setValid = false;
         }
      }
      if (setValid) {
         //go through each equation and see what is applicable
         //EquationMap::const_iterator map_i = set_i->second->find(name);
         EquationMap::const_iterator map_i = set_i->second;

         for (expr_pair_v::const_iterator vec_i = map_i->second.begin();
            vec_i != map_i->second.end();
            ++vec_i)
         {
            if (vec_i->first) {
               try {
                  Tuple evalContext =
                     vec_i->first.evaluate(m_i, k);
                  //std::cout << "guard:" << std::endl;
                  //evalContext.print(m_i, std::cout, c);
                  if (tupleApplicable(m_i, evalContext, k) && booleanTrue(m_i, vec_i->first, k)) {
                     applicable.push_back(
                        ApplicableTuple(evalContext, vec_i->second));
                  }
               } catch (InvalidGuard& e) {
               }
            } else {
               applicable.push_back(
                  ApplicableTuple(
                     Tuple(), vec_i->second));
            }
         }
      }
   }

   if (applicable.size() == 0) {
      return TypedValue(
         Special(Special::UNDEF),
         m_i.typeRegistry().indexSpecial());
   } else if (applicable.size() == 1) {
      return applicable.front().get<1>()->evaluate(m_i, k).first;
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
      return TypedValue(
         Special(Special::UNDEF),
         m_i.typeRegistry().indexSpecial());
   }

   for (applicable_list::const_iterator iter = applicable.begin();
      iter != applicable.end(); ++iter)
   {
      if (bestIter->get<1>() != iter->get<1>() &&
         !tupleRefines(m_i, bestIter->get<0>(), iter->get<0>()))
      {
         return TypedValue(
            Special(Special::MULTIDEF),
            m_i.typeRegistry().indexSpecial());
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
}

}
