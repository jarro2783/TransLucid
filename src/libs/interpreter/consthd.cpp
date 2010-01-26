#include <tl/consthd.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>

namespace TransLucid {

namespace ConstHD {

const char *UChar::name =     "uchar";
const char *Intmp::name =     "intmp";

TaggedValue UChar::operator()(const Tuple& k) {
   size_t valueindex = get_dimension_index(m_system, "text").get_ui();
   Tuple::const_iterator value = k.find(valueindex);

   if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING) {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         TYPE_INDEX_SPECIAL), k);
   }

   const ustring_t& s = value->second.value<String>().value();
   //return TaggedValue(m_i.typeRegistry().findType("uchar")->parse(s.value(), k, m_i), k);
   if (s.length() != 1) {
      return TaggedValue(TypedValue(Special(Special::CONST), TYPE_INDEX_SPECIAL), k);
   }
   return TaggedValue(TypedValue(Char(s[0]), TYPE_INDEX_UCHAR), k);
}

TaggedValue Intmp::operator()(const Tuple& k) {
   Tuple::const_iterator value = k.find(DIM_TEXT);

   if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING) {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         TYPE_INDEX_SPECIAL), k);
   }

   return TaggedValue(TypedValue(TransLucid::Intmp(
         mpz_class(value->second.value<String>().value().raw())),
      TYPE_INDEX_INTMP), k);
}

} //namespace ConstHD

} //namespace TransLucid
