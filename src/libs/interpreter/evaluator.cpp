#include <tl/evaluator.hpp>
#include <tl/interpreter.hpp>
#include <boost/assign.hpp>
#include <tl/exception.hpp>
#include <tl/range.hpp>

using boost::assign::list_of;

namespace TransLucid {

using namespace AST;

namespace {
   struct ContextV : public Data {
      ContextV() {
      }

      ContextV(const Tuple& c)
      : c(c)
      {
      }

      Tuple c;
   };

   struct ValueV : public Data {
      ValueV(const TypedValue& v, const Tuple& c)
      : value(v), c(c)
      {}

      ValueV(const ValueContext& vc)
      : value(vc.first), c(vc.second)
      {
      }

      TypedValue value;
      Tuple c;
   };

   struct PairV : public Data {
      PairV(const TypedValue& lhs, const TypedValue& rhs)
      : lhs(lhs), rhs(rhs)
      {
      }

      TypedValue lhs;
      TypedValue rhs;
   };

   inline ValueV* makeValue(Data *d) {
      return dynamic_cast<ValueV*>(d);
   }

   inline ContextV* makeContext(Data *d) {
      return dynamic_cast<ContextV*>(d);
   }

   inline PairV* makePair(Data *d) {
      return dynamic_cast<PairV*>(d);
   }

   //k dagger c
   void perturb(tuple_t& k, const Tuple& c) {
      Tuple::const_iterator iter = c.begin();
      for (; iter != c.end(); ++iter) {
         k[iter->first] = iter->second;
      }
   }
}

Evaluator::Evaluator(Interpreter& i)
: m_interpreter(i), m_registry(i.typeRegistry()), m_dims(i.dimTranslator())
{
}

ValueContext Evaluator::evaluate(Expr *e) {
   //need to start with the default context
   ContextV *c = new ContextV;
   ValueV *v = makeValue(e->visit(this, c));
   delete c;
   std::pair<TypedValue, Tuple> result = std::make_pair(v->value, v->c);
   delete v;
   return result;
}

ValueContext Evaluator::evaluate(AST::Expr* e, const Tuple& context) {
   ContextV *c = new ContextV(context);
   ValueV *v = makeValue(e->visit(this, c));
   delete c;
   std::pair<TypedValue, Tuple> result = std::make_pair(v->value, v->c);
   delete v;
   return result;
}

size_t Evaluator::lookupDim(const TypedValue& v) {
   size_t i;
   if (v.index() == m_registry.indexDimension()) {
      const Dimension& d = v.value<Dimension>();
      i = d.value();
   } else {
      i = m_dims.insert(v);
   }

   return i;
}

TypedValue Evaluator::makeSpecial(Special::Value value) {
   return TypedValue(Special(value), m_registry.indexSpecial());
}

TypedValue Evaluator::makeBoolean(bool b) {
   return TypedValue(Boolean(b), m_registry.indexBoolean());
}

TypedValue Evaluator::makeDimension(size_t v) {
   return TypedValue(Dimension(v), m_registry.indexDimension());
}

Data* Evaluator::visitAtExpr(AtExpr *e, Data *data) {
   ValueV *result = makeValue(e->e1->visit(this, data));

   Data *retval;

   size_t timeDim = m_dims.lookup("time");

   if (result->value.index() != m_registry.indexTuple()) {
      retval = new ValueV(makeSpecial(Special::TYPEERROR), makeContext(data)->c);
   } else {
      ContextV *current = makeContext(data);
      ContextV *c = new ContextV;
      const Tuple& rhsTuple = result->value.value<Tuple>();
      tuple_t changed;
      if (e->relative) {
         changed = current->c.tuple();
         perturb(changed, rhsTuple);
         c->c = changed;
      } else {
         c->c = rhsTuple.tuple();
      }

      Tuple::const_iterator lhsdim = c->c.find(timeDim);
      Tuple::const_iterator rhsdim = current->c.find(timeDim);
      if (lhsdim != c->c.end() &&
         rhsdim != current->c.end() &&
         lhsdim->second.value<Intmp>().value() <=
            rhsdim->second.value<Intmp>().value())
      {
         retval = e->e2->visit(this, c);
      } else {
         retval = new ValueV(makeSpecial(Special::ACCESS), current->c);
      }
      delete c;
   }

   delete result;

   return retval;
}

Data* Evaluator::visitBinaryOpExpr(BinaryOpExpr *e, Data *data) {
   ContextV *c = makeContext(data);
   switch (e->op.assoc) {
      case Parser::ASSOC_LEFT:
      case Parser::ASSOC_RIGHT:
      case Parser::ASSOC_NON:
      case Parser::ASSOC_VARIABLE:
      {
         bool special = false;
         Special::Value specialValue = Special::ERROR;
         std::vector<TypedValue> values;

         BOOST_FOREACH(Expr *expr, e->operands) {
            std::auto_ptr<ValueV> v(makeValue(expr->visit(this, data)));
            values.push_back(v->value);
         }

         type_vec inTypes;

         BOOST_FOREACH(const TypedValue& tv, values) {
            if (tv.index() == m_registry.indexSpecial()) {
               Special::Value newspec = tv.value<Special>().value();
               if (special) {
                  if (newspec < specialValue) {
                     specialValue = newspec;
                  }
               } else {
                  special = true;
                  specialValue = newspec;
               }
            } else {
               inTypes.push_back(tv.index());
            }
         }

         if (special) {
            return new ValueV(makeSpecial(specialValue), c->c);
         } else {
            if (e->op.assoc == Parser::ASSOC_VARIABLE) {
               //check if everything is equal to the first value
               type_index index = inTypes.at(0);
               if (size_t(std::count(inTypes.begin(), inTypes.end(), index))
                  < inTypes.size())
               {
                  return new ValueV(makeSpecial(Special::TYPEERROR), c->c);
               } else {
                  OpFunction op = m_registry.findVariadicOp(e->op.op, index);
                  TypedValue v = op(values, c->c);
                  return new ValueV(v, c->c);
               }
            } else {
               OpFunction op = m_registry.findOp(e->op.op, inTypes);
               TypedValue result = op(values, c->c);
               return new ValueV(result, c->c);
            }
         }
      }
      break;

      case Parser::ASSOC_COMPARISON:
      {
         //evaluate all the arguments with as a op b && b op c etc
         //op must return bool otherwise special<type> will be
         //returned, if any value is special it will be returned
         size_t rhsIndex = 1;
         bool evalTrue = true;
         std::auto_ptr<ValueV> lhs(makeValue(e->operands.at(0)->visit(this, data)));
         std::auto_ptr<ValueV> rhs;
         if (lhs->value.index() == m_registry.indexSpecial()) {
            return lhs.release();
         }
         while (rhsIndex != e->operands.size() && evalTrue) {
            rhs.reset(makeValue(e->operands.at(rhsIndex)->visit(this, data)));

            if (rhs->value.index() == m_registry.indexSpecial()) {
               return rhs.release();
            }

            OpFunction op = m_registry.findOp(e->op.op,
               list_of(lhs->value.index())(rhs->value.index()));

            TypedValue result = op(list_of(lhs->value)(rhs->value), c->c);

            if (result.index() != m_registry.indexBoolean()) {
               if (result.index() == m_registry.indexSpecial()) {
                  return new ValueV(result, c->c);
               } else {
                  return new ValueV(makeSpecial(Special::TYPEERROR), c->c);
               }
            }
            evalTrue = result.value<Boolean>();

            lhs = rhs;
            ++rhsIndex;
         }
         return new ValueV(makeBoolean(evalTrue), c->c);
      }
      break;
   }

   //if we get here this is an error
   throw InternalError("End of evaluate BinaryOpExpr reached");
}

Data* Evaluator::visitBooleanExpr(BooleanExpr *e, Data *data) {
   return new ValueV(
      TypedValue(Boolean(e->value), m_registry.indexBoolean()),
      makeContext(data)->c);
}

Data* Evaluator::visitConstantExpr(ConstantExpr *e, Data *d) {
   const TypeManager *m = m_registry.findType(e->name);
   ContextV *c = makeContext(d);

   if (m != 0) {
      return new ValueV(m->parse(e->value, c->c, m_interpreter), c->c);
   } else {
      return new ValueV(makeSpecial(Special::TYPEERROR), c->c);
   }
}

Data *Evaluator::visitBuildTupleExpr(BuildTupleExpr *e, Data *d) {

   ContextV *c = makeContext(d);
   tuple_t tuple1;

   //build a tuple out of the list of pairs
   BOOST_FOREACH(Expr *expr, e->values) {
      PairV *result = makePair(expr->visit(this, d));

      size_t lhs;
      if (result->lhs.index() == m_registry.indexDimension()) {
         const Dimension& d = result->lhs.value<Dimension>();
         lhs = d.value();
      } else {
         lhs = m_dims.insert(result->lhs);
      }

      tuple1.insert(
         std::make_pair(lhs, result->rhs)
      );
      delete result;
   }

   return new ValueV(
      TypedValue(Tuple(tuple1), m_registry.indexTuple()),
      c->c);
}

Data *Evaluator::visitConvertExpr(SpecialOpsExpr *e, Data *d) {

   ValueV *value = makeValue(e->e->visit(this, d));
   ContextV *c = makeContext(d);

   const TypeManager *type = m_registry.findType(e->value);

   if (type == 0) {
      return new ValueV(makeSpecial(Special::TYPEERROR), makeContext(d)->c);
   } else {
      ConvertFunction convert = m_registry.findConverter(type->index(), value->value.index());
      return new ValueV(convert(value->value, c->c), c->c);
   }
}

Data* Evaluator::visitDimensionExpr(DimensionExpr *e, Data *d) {
   size_t v = m_dims.lookup(e->value);
   return new ValueV(makeDimension(v), makeContext(d)->c);
}

Data* Evaluator::visitHashExpr(HashExpr *e, Data *d) {
   ValueV *v = makeValue(e->e->visit(this, d));
   ContextV *c = makeContext(d);

   if (v->value.index() == m_registry.indexSpecial()) {
      return v;
   } else {
      size_t dim = lookupDim(v->value);
      Tuple::const_iterator iter;
      if (dim != 0 && (iter = c->c.find(dim)) != c->c.end()) {
         delete v;
         return new ValueV(iter->second, c->c);
      } else {
         delete v;
         return new ValueV(TypedValue(Special(Special::DIMENSION), m_registry.indexSpecial()), c->c);
      }
   }
}

Data *Evaluator::visitIdentExpr(IdentExpr* e, Data *d) {
   ContextV *c = makeContext(d);

   std::pair<bool, TypedValue> cached = m_interpreter.warehouse().lookupCalc(e->id, c->c);

   //if it doesn't exist just compute it
   if (cached.first) {
      //if type is calc then BFOAT and loop
      //otherwise return the result
      if (cached.second.index() == m_registry.indexCalc()) {
         return new ValueV(makeSpecial(Special::LOOP), c->c);
      } else {
         return new ValueV(cached.second, c->c);
      }

   } else {

      //Variable *v = m_interpreter.lookupVariable(e->id);
      #warning interpreter.get

      Variable *v = 0;

      if (v) {
         ValueContext vc = (*v)(c->c);
         m_interpreter.warehouse().add(e->id, vc.first, c->c);
         return new ValueV(vc);
      } else {
         return new ValueV(makeSpecial(Special::UNDEF), c->c);
      }
   }
}

Data *Evaluator::visitIfExpr(IfExpr* e, Data *d) {
   ContextV *c = makeContext(d);
   ValueV *ifcond = makeValue(e->condition->visit(this, d));

   type_index condIndex = ifcond->value.index();

   ValueV *result = 0;

   if (condIndex == m_registry.indexSpecial()) {
      result = ifcond;
      ifcond = 0;
   } else if (condIndex == m_registry.indexBoolean()) {
      const Boolean& b = ifcond->value.value<Boolean>();

      if (b) {
         result = makeValue(e->then->visit(this, d));
      } else {
         //run the elsifs and else
         bool handled = false;

         std::list<Expr*>::const_iterator iter = e->elsifs.begin();
         while (iter != e->elsifs.end() && !handled) {
            std::auto_ptr<ValueV> cond(makeValue((*iter)->visit(this, d)));

            type_index index = cond->value.index();

            if (index == m_registry.indexSpecial()) {
               result = cond.release();
               handled = true;
            } else if (index == m_registry.indexBoolean()) {
               const Boolean& bcond = cond->value.value<Boolean>();
               ++iter;
               if (bcond) {
                  result = makeValue((*iter)->visit(this, d));
                  handled = true;
               }
            } else {
               result = new ValueV(makeSpecial(Special::TYPEERROR), c->c);
               handled = true;
            }

            ++iter;
         }

         if (!handled) {
            result = makeValue(e->else_->visit(this, d));
         }
      }

   } else {
      result = new ValueV(makeSpecial(Special::TYPEERROR), c->c);
   }

   delete ifcond;

   return result;
}

Data *Evaluator::visitIntegerExpr(IntegerExpr *e, Data *d) {
   ContextV *c = makeContext(d);
   return new ValueV(TypedValue(Intmp(e->m_value), m_registry.indexIntmp()), c->c);
}

Data *Evaluator::visitIsSpecialExpr(SpecialOpsExpr *e, Data *d) {
   ContextV *c = makeContext(d);
   ValueV *v = makeValue(e->e->visit(this, d));

   Special::Value value = Special::stringToValue(e->value);

   bool result;

   if (v->value.index() == m_registry.indexSpecial() &&
      value == v->value.value<Special>().value())
   {
      result = true;
   } else {
      result = false;
   }

   delete v;

   return new ValueV(makeBoolean(result), c->c);
}

Data *Evaluator::visitIsTypeExpr(SpecialOpsExpr *e, Data *d) {
   const TypeManager *m = m_registry.findType(e->value);

   ValueV *v = makeValue(e->e->visit(this, d));

   bool result = false;

   if (m != 0 && v->value.index() == m->index()) {
      result = true;
   }

   return new ValueV(makeBoolean(result), makeContext(d)->c);
}

Data *Evaluator::visitPairExpr(PairExpr* e, Data *d) {
   ValueV *lhs = makeValue(e->lhs->visit(this, d));
   ValueV *rhs = makeValue(e->rhs->visit(this, d));

   PairV *result = new PairV(lhs->value, rhs->value);

   delete lhs;
   delete rhs;

   return result;
}

Data *Evaluator::visitRangeExpr(RangeExpr *e, Data *d) {
   ContextV *c = makeContext(d);
   std::auto_ptr<ValueV> lhs(
      e->lhs != 0 ? makeValue(e->lhs->visit(this, d)) : 0);
   std::auto_ptr<ValueV> rhs(
      e->rhs != 0 ? makeValue(e->rhs->visit(this, d)) : 0);

   if ((lhs.get() && lhs->value.index() != m_registry.indexIntmp())
      || (rhs.get() && rhs->value.index() != m_registry.indexIntmp()))
   {
      return new ValueV(makeSpecial(Special::TYPEERROR), c->c);
   }

   //this value() thing is the WOAT
   return new ValueV(TypedValue(
      Range(
         lhs.get() != 0 ? &lhs->value.value<Intmp>().value() : 0,
         rhs.get() != 0 ? &rhs->value.value<Intmp>().value() : 0),
      m_registry.indexRange()), c->c);
}

Data *Evaluator::visitUnaryExpr(UnaryExpr* e, Data *d) {
   std::auto_ptr<ValueV> v(makeValue(e->e->visit(this, d)));

   OpFunction op = m_registry.findOp(e->op.op, list_of(v->value.index()));

   ContextV *c = makeContext(d);
   return new ValueV(op(list_of(v->value), c->c), c->c);
}

}
