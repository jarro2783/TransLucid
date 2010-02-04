#include <tl/compiled_functors.hpp>
#include <boost/assign/list_of.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>

namespace TransLucid {

namespace CompiledFunctors {

namespace {

template <typename T, typename V>
inline void contextInsert(
   tuple_t& k,
   HD *h,
   const ustring_t& dim,
   const V& value,
   size_t index)
{
   k[get_dimension_index(h, dim)] = TypedValue(T(value), index);
}

inline void contextInsert(
   tuple_t& k,
   Interpreter& i,
   const ustring_t& dim,
   const TypedValue& v)
{
   k[get_dimension_index(&i, dim)] = v;
}

template <typename T>
class TupleInserter {
   public:
   TupleInserter(tuple_t& k, HD *h, size_t index)
   : m_k(k), m_h(h), m_index(index)
   {
   }

   template <typename V>
   const TupleInserter<T>& operator()(const ustring_t& dim, const V& v) const {
      m_k[get_dimension_index(m_h, dim)] = TypedValue(T(v), m_index);
      return *this;
   }

   private:
   tuple_t& m_k;
   HD *m_h;
   size_t m_index;
};

template <typename T>
TupleInserter<T> insert_tuple(tuple_t& k, HD *h, size_t index) {
   return TupleInserter<T>(k, h, index);
}

}

TaggedValue AtAbsolute::operator()(const Tuple& k) {
   TaggedValue kp = (*e1)(k);
   if (kp.first.index() != TYPE_INDEX_TUPLE) {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         TYPE_INDEX_SPECIAL), k);
   } else {
      return (*e2)(kp.first.value<Tuple>());
   }
}

TaggedValue AtRelative::operator()(const Tuple& k) {
   tuple_t kNew = k.tuple();
   TaggedValue kp = (*e1)(k);
   if (kp.first.index() != TYPE_INDEX_TUPLE) {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         TYPE_INDEX_SPECIAL), k);
   } else {
      BOOST_FOREACH(tuple_t::value_type v, kp.first.value<Tuple>().tuple()) {
         kNew[v.first] = v.second;
      }
      return (*e2)(Tuple(kNew));
   }
}

TaggedValue BoolConst::operator()(const Tuple& k) {
   //return boost::assign::list_of(ValueContext(
   //   TypedValue(TransLucid::Boolean(m_value), i.typeRegistry().indexBoolean()), Tuple()));
   tuple_t kp = k.tuple();
   insert_tuple<String>(kp, &m_system, TYPE_INDEX_USTRING)
      ("id", "CONST")
      ("type", "bool")
      ("text", m_value ? "true" : "false")
   ;

   return m_system(Tuple(kp));
}

TaggedValue BuildTuple::operator()(const Tuple& k) {
   tuple_t kp;
   BOOST_FOREACH(HD* h, m_elements) {
      TaggedValue v = (*h)(k);
      if (v.first.index() != TYPE_INDEX_PAIR) {
         return TaggedValue(TypedValue(Special(Special::TYPEERROR),
            TYPE_INDEX_SPECIAL), k);
      } else {
         const PairType& p = v.first.value<PairType>();

         if (p.first().index() == TYPE_INDEX_DIMENSION) {
            kp[p.first().value<TransLucid::Dimension>().value()] = p.second();
         } else {
            kp[get_dimension_index(&m_system, p.first())] = p.second();
         }
      }
   }
   return TaggedValue(TypedValue(Tuple(kp), TYPE_INDEX_TUPLE), k);
}

TaggedValue Constant::operator()(const Tuple& k) {
   //evaluate CONST
   tuple_t kp = k.tuple();
   //kp[m_system.dimTranslator().lookup("id")] =
   //   TypedValue(String("CONST"), m_system.typeRegistry().indexString());
   //contextInsertString(kp, m_system, "id", "CONST", indexString);
   //contextInsertString(kp, m_system, "type", m_type, indexString);
   //contextInsertString(kp, m_system, "text", m_text, indexString);
   insert_tuple<String>(kp, &m_system, TYPE_INDEX_USTRING)
      ("id", "CONST")
      ("type", m_type)
      ("text", m_text)
   ;

   return m_system(Tuple(kp));
}

TaggedValue Convert::operator()(const Tuple& context) {
   //going away
   return TaggedValue();
}

TaggedValue Dimension::operator()(const Tuple& k) {
   size_t id = get_dimension_index(&m_system, m_name);
   return TaggedValue(
      TypedValue(TransLucid::Dimension(id),
         TYPE_INDEX_DIMENSION),
      k);
}

TaggedValue Hash::operator()(const Tuple& k) {
   TaggedValue r = (*m_e)(k);
   size_t index;
   if (r.first.index() == TYPE_INDEX_DIMENSION) {
      index = r.first.value<TransLucid::Dimension>().value();
   } else {
      index = get_dimension_index(m_system, r.first);
   }

   Tuple::const_iterator iter = k.find(index);
   if (iter != k.end()) {
      return TaggedValue(iter->second, k);
   } else {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         TYPE_INDEX_SPECIAL), k);
   }
}

TaggedValue Ident::operator()(const Tuple& k) {
   std::cout << "ident: " << m_name << std::endl;
   tuple_t kp = k.tuple();

   insert_tuple<String>(kp, m_system, TYPE_INDEX_USTRING)
      ("id", m_name)
      ;
   return (*m_system)(Tuple(kp));
}

TaggedValue If::operator()(const Tuple& k) {
   TaggedValue cond = (*m_condition)(k);
   TypedValue& condv = cond.first;

   if (condv.index() == TYPE_INDEX_SPECIAL) {
      return cond;
   } else if (condv.index() == TYPE_INDEX_BOOL) {
      const Boolean& b = condv.value<Boolean>();

      if (b) {
         return (*m_then)(k);
         //result = makeValue(e->then->visit(this, d));
      } else {
         //run the elsifs and else
         std::list<HD*>::const_iterator iter = m_elsifs.begin();
         while (iter != m_elsifs.end()) {
            //std::auto_ptr<ValueV> cond(makeValue((*iter)->visit(this, d)));
            TaggedValue cond = (*iter)->operator()(k);

            type_index index = cond.first.index();

            if (index == TYPE_INDEX_SPECIAL) {
               return cond;
            } else if (index == TYPE_INDEX_BOOL) {
               const Boolean& bcond = cond.first.value<Boolean>();
               ++iter;
               if (bcond) {
                  return (*iter)->operator()(k);
               }
            } else {
               return TaggedValue(TypedValue(Special(Special::TYPEERROR),
                  TYPE_INDEX_SPECIAL), k);
            }

            ++iter;
         }

         return (*m_else)(k);
      }
   } else {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         TYPE_INDEX_SPECIAL), k);
   }
}

TaggedValue Integer::operator()(const Tuple& k) {
   // @[id : CONST, type : @[id : DEFAULTINT], value : m_value]
   tuple_t kp = k.tuple();
   TupleInserter<String> inserter(kp, &m_system, TYPE_INDEX_USTRING);
   //contextInsert<String>(kp, m_system, "id", "DEFAULTINT", m_system.typeRegistry().indexString());
   kp[DIM_ID] = generate_string("DEFAULTINT");
   //TaggedValue defaultint = m_system(Tuple(kp));

   kp[DIM_TEXT] = generate_string(m_value.get_str(10));

   return m_system(Tuple(kp));

   #if 0
   //kp = k.tuple();
   inserter("id", "CONST");
   contextInsert(kp, m_system, "type", defaultint.first);
   contextInsert(kp, m_system, "text", generate_string(m_value.get_str(10)));
   return m_system(Tuple(kp));
   #endif
}

TaggedValue IsSpecial::operator()(const Tuple& context) {
   //this is going away, but to stop warnings
   return TaggedValue();
}

TaggedValue IsType::operator()(const Tuple& context) {
   //this is going away, but to stop warnings
   return TaggedValue();
}

TaggedValue Operation::operator()(const Tuple& k) {
   tuple_t kp = k.tuple();

   TupleInserter<String> insert(kp, &m_system, TYPE_INDEX_USTRING);
   insert("id", "FUN");

   int i = 0;
   std::ostringstream os;
   BOOST_FOREACH(HD *h, m_operands) {
      os.str("arg");
      os << i;
      kp[get_dimension_index(h, os.str())] = (*h)(k).first;
      ++i;
   }

   return m_system(Tuple(kp));
}

TaggedValue Pair::operator()(const Tuple& k) {

   TaggedValue l = (*m_lhs)(k);
   TaggedValue r = (*m_rhs)(k);

   return TaggedValue(TypedValue(PairType((*m_lhs)(k).first, (*m_rhs)(k).first),
      TYPE_INDEX_PAIR), k);
}

TaggedValue UnaryOp::operator()(const Tuple& context) {
   //this will probably also go away
   return TaggedValue();
}

} //namespace SetLazyEvaluator

} //namespace TransLucid
