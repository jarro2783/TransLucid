#include <tl/dimtranslator.hpp>

namespace TransLucid {

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
