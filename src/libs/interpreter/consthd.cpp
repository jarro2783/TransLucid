#include <tl/consthd.hpp>
#include <tl/builtin_types.hpp>

namespace TransLucid {

namespace ConstHD {

const char *UChar::name = "uchar";

TaggedValue UChar::operator()(const Tuple& k) {
   size_t valueindex = m_i.dimTranslator().lookup("text");
   Tuple::const_iterator value = k.find(valueindex);

   if (value == k.end() || value->second.index() != m_i.typeRegistry().indexString()) {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         m_i.typeRegistry().indexSpecial()), k);
   }

   const ustring_t& s = value->second.value<String>().value();
   //return TaggedValue(m_i.typeRegistry().findType("uchar")->parse(s.value(), k, m_i), k);
   if (s.length() != 1) {
      return TaggedValue(TypedValue(Special(Special::CONST),
         m_i.typeRegistry().indexSpecial()), k);
   }
   return TaggedValue(TypedValue(Char(s[0]), m_i.typeRegistry().indexChar()), k);
}

void UChar::addExpr(const Tuple& k, HD *h) {
}

} //namespace ConstHD

} //namespace TransLucid
