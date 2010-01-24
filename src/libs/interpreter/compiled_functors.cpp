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
   Interpreter& i,
   const ustring_t& dim,
   const V& value,
   size_t index)
{
   k[i.dimTranslator().lookup(dim)] = TypedValue(T(value), index);
}

inline void contextInsert(
   tuple_t& k,
   Interpreter& i,
   const ustring_t& dim,
   const TypedValue& v)
{
   k[i.dimTranslator().lookup(dim)] = v;
}

template <typename T>
class TupleInserter {
   public:
   TupleInserter(tuple_t& k, Interpreter& i, size_t index)
   : m_k(k), m_i(i), m_index(index)
   {
   }

   template <typename V>
   const TupleInserter<T>& operator()(const ustring_t& dim, const V& v) const {
      m_k[m_i.dimTranslator().lookup(dim)] = TypedValue(T(v), m_index);
      return *this;
   }

   private:
   tuple_t& m_k;
   Interpreter& m_i;
   size_t m_index;
};

template <typename T>
TupleInserter<T> insert_tuple(tuple_t& k, Interpreter& i, size_t index) {
   return TupleInserter<T>(k, i, index);
}

}

TaggedValue AtAbsolute::operator()(const Tuple& k) {
   TaggedValue kp = (*e1)(k);
   if (kp.first.index() != m_system.typeRegistry().indexTuple()) {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         m_system.typeRegistry().indexSpecial()), k);
   } else {
      return (*e2)(kp.first.value<Tuple>());
   }
}

TaggedValue AtRelative::operator()(const Tuple& k) {
   tuple_t kNew = k.tuple();
   TaggedValue kp = (*e1)(k);
   if (kp.first.index() != m_system.typeRegistry().indexTuple()) {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         m_system.typeRegistry().indexSpecial()), k);
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
   #warning maybe there is a better way to do this
   size_t indexString = m_system.typeRegistry().indexString();
   tuple_t kp = k.tuple();
   insert_tuple<String>(kp, m_system, indexString)
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
      if (v.first.index() != m_system.typeRegistry().indexPair()) {
         return TaggedValue(TypedValue(Special(Special::TYPEERROR),
            m_system.typeRegistry().indexSpecial()), k);
      } else {
         const PairType& p = v.first.value<PairType>();
         kp[m_system.dimTranslator().lookup(p.first())] = p.second();
      }
   }
   return TaggedValue(TypedValue(Tuple(kp), m_system.typeRegistry().indexTuple()), k);
}

TaggedValue Constant::operator()(const Tuple& k) {
   //evaluate CONST
   size_t indexString = m_system.typeRegistry().indexString();
   tuple_t kp = k.tuple();
   //kp[m_system.dimTranslator().lookup("id")] =
   //   TypedValue(String("CONST"), m_system.typeRegistry().indexString());
   //contextInsertString(kp, m_system, "id", "CONST", indexString);
   //contextInsertString(kp, m_system, "type", m_type, indexString);
   //contextInsertString(kp, m_system, "text", m_text, indexString);
   insert_tuple<String>(kp, m_system, indexString)
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
   return TaggedValue(
      TypedValue(TransLucid::Dimension(m_system.dimTranslator().lookup(m_name)),
         m_system.typeRegistry().indexDimension()),
      k);
}

TaggedValue Hash::operator()(const Tuple& k) {
   size_t index = m_system.dimTranslator().lookup((*m_e)(k).first);
   Tuple::const_iterator iter = k.find(index);
   if (iter != k.end()) {
      return TaggedValue(iter->second, k);
   } else {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         m_system.typeRegistry().indexSpecial()), k);
   }
}

TaggedValue Ident::operator()(const Tuple& k) {
   tuple_t kp = k.tuple();

   insert_tuple<String>(kp, m_system, m_system.typeRegistry().indexString())
      ("id", m_name)
      ;
   return m_system(Tuple(kp));
}

TaggedValue If::operator()(const Tuple& k) {
   TaggedValue cond = (*m_condition)(k);
   TypedValue& condv = cond.first;

   if (condv.index() == m_system.typeRegistry().indexSpecial()) {
      return cond;
   } else if (condv.index() == m_system.typeRegistry().indexBoolean()) {
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

            if (index == m_system.typeRegistry().indexSpecial()) {
               return cond;
            } else if (index == m_system.typeRegistry().indexBoolean()) {
               const Boolean& bcond = cond.first.value<Boolean>();
               ++iter;
               if (bcond) {
                  return (*iter)->operator()(k);
               }
            } else {
               return TaggedValue(TypedValue(Special(Special::TYPEERROR),
                  m_system.typeRegistry().indexSpecial()), k);
            }

            ++iter;
         }

         return (*m_else)(k);
      }
   } else {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         m_system.typeRegistry().indexSpecial()), k);
   }
}

TaggedValue Integer::operator()(const Tuple& k) {
   // @[id : CONST, type : @[id : DEFAULTINT], value : m_value]
   tuple_t kp = k.tuple();
   TupleInserter<String> inserter(kp, m_system, TYPE_INDEX_USTRING);
   //contextInsert<String>(kp, m_system, "id", "DEFAULTINT", m_system.typeRegistry().indexString());
   kp[DIM_ID] = generate_string("DEFAULTINT");
   TaggedValue defaultint = m_system(Tuple(kp));

   //kp = k.tuple();
   inserter("id", "CONST");
   contextInsert(kp, m_system, "type", defaultint.first);
   contextInsert<Intmp>(kp, m_system, "value", m_value,
      m_system.typeRegistry().indexIntmp());
   return m_system(Tuple(kp));
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

   TupleInserter<String> insert(kp, m_system, m_system.typeRegistry().indexString());
   insert("id", "FUN");

   int i = 0;
   std::ostringstream os;
   BOOST_FOREACH(HD *h, m_operands) {
      os.str("arg");
      os << i;
      kp[m_system.dimTranslator().lookup(os.str())] = (*h)(k).first;
      ++i;
   }

   return m_system(Tuple(kp));
}

TaggedValue Pair::operator()(const Tuple& k) {
   return TaggedValue(TypedValue(PairType((*m_lhs)(k).first, (*m_rhs)(k).first),
      m_system.typeRegistry().indexPair()), k);
}

TaggedValue UnaryOp::operator()(const Tuple& context) {
   //this will probably also go away
   return TaggedValue();
}

} //namespace SetLazyEvaluator

} //namespace TransLucid
