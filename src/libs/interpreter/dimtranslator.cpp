#include <tl/dimtranslator.hpp>
#include <boost/assign/list_of.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid {

DimensionTranslator::DimensionTranslator()
: m_nextIndex(RESERVED_INDEX_LAST),
   m_namedDims(boost::assign::map_list_of(ustring_t("type"), int(DIM_TYPE))
              ("text", DIM_TEXT)
              ("name", DIM_NAME)
              ("id", DIM_ID)
              )
{
}

size_t DimensionTranslator::lookup(const ustring_t& name) {
   std::pair<ustring_size_map::iterator,bool> result =
      m_namedDims.insert(std::make_pair(name, m_nextIndex));
   if (result.second) {
      ++m_nextIndex;
   }
   return result.first->second;
   #if 0
   ustring_size_map::iterator iter = m_namedDims.find(name);
   if (iter == m_namedDims.end()) {
      return m_namedDims.insert(std::make_pair(name, m_nextIndex++)).first->second;
   } else {
      return iter->second;
   }
   #endif
}

size_t DimensionTranslator::lookup(const TypedValue& value) {
   std::pair<ustring_type_map::iterator,bool> result =
      m_typedDims.insert(std::make_pair(value, m_nextIndex));
   if (result.second) {
      ++m_nextIndex;
   }
   return result.first->second;
   #if 0
   ustring_type_map::iterator iter = m_typedDims.find(value);
   if (iter == m_typedDims.end()) {
      return m_typedDims.insert(std::make_pair(value, m_nextIndex++)).first->second;
   } else {
      return iter->second;
   }
   #endif
}

}
