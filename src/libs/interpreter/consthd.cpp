#include <tl/consthd.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>

namespace TransLucid {

namespace {

   std::string u32_to_ascii(const u32string& s) {
      std::string r;

      BOOST_FOREACH(char32_t c, s) {
         if (c > 0x7F) {
            throw "character not ascii";
         } else {
            r += c;
         }
      }
      return r;
   }

}

namespace ConstHD {

const char32_t *UChar::name =     U"uchar";
const char32_t *Intmp::name =     U"intmp";
const char32_t *UString::name =   U"ustring";

TaggedValue UChar::operator()(const Tuple& k) {
   size_t valueindex = get_dimension_index(m_system, U"text");
   Tuple::const_iterator value = k.find(valueindex);

   if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING) {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         TYPE_INDEX_SPECIAL), k);
   }

   const u32string& s = value->second.value<String>().value();
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

   try {
      return TaggedValue(TypedValue(TransLucid::Intmp(
         mpz_class(u32_to_ascii(value->second.value<String>().value())) ),
         TYPE_INDEX_INTMP), k);
   } catch (...) {
      return TaggedValue(TypedValue(Special(Special::CONST),
         TYPE_INDEX_SPECIAL), k);
   }
}

TaggedValue UString::operator()(const Tuple& k) {
   Tuple::const_iterator value = k.find(DIM_TEXT);

   if (value == k.end() || value->second.index() != TYPE_INDEX_USTRING) {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
         TYPE_INDEX_SPECIAL), k);
   }

   return TaggedValue(value->second, k);
}

} //namespace ConstHD

} //namespace TransLucid
