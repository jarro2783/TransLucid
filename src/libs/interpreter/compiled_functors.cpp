#include <tl/compiled_functors.hpp>
#include <boost/assign/list_of.hpp>
#include <tl/builtin_types.hpp>

namespace TransLucid {

namespace CompiledFunctors {

namespace {

inline void contextInsertString(
   tuple_t& k,
   Interpreter& i,
   ustring_t& dim,
   ustring_t& value,
   size_t index)
{
   k[i.dimTranslator().lookup(dim)] = TypedValue(String(value), index);
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

TaggedValue AtAbsolute::operator()(const Tuple& context) {
}

TaggedValue AtRelative::operator()(const Tuple& context) {
}

TaggedValue BoolConst::operator()(const Tuple& context) {
   //return boost::assign::list_of(ValueContext(
   //   TypedValue(TransLucid::Boolean(m_value), i.typeRegistry().indexBoolean()), Tuple()));
}

TaggedValue BuildTuple::operator()(const Tuple& context) {
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

TaggedValue Hash::operator()(const Tuple& context) {
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
      }
   } else {
      return TaggedValue(TypedValue(Special(Special::TYPEERROR),
         m_system.typeRegistry().indexSpecial()), k);
   }
}

TaggedValue Integer::operator()(const Tuple& context) {
   // @ [id : CONST, value : [id : DEFAULTINT]]
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

   m_system(Tuple(kp));
}

TaggedValue Pair::operator()(const Tuple& context) {
}

TaggedValue UnaryOp::operator()(const Tuple& context) {
   //this will probably also go away
   return TaggedValue();
}

} //namespace SetLazyEvaluator

} //namespace TransLucid
