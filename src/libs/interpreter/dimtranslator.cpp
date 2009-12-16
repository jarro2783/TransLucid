#include <tl/dimtranslator.hpp>

namespace TransLucid {

size_t DimensionTranslator::insert(const ustring_t& name) {
   ustring_size_map::iterator iter = m_namedDims.find(name);
   if (iter == m_namedDims.end()) {
      return m_namedDims.insert(std::make_pair(name, m_nextIndex++)).first->second;
   } else {
      return iter->second;
   }
}

size_t DimensionTranslator::insert(const TypedValue& value) {
   ustring_type_map::iterator iter = m_typedDims.find(value);
   if (iter == m_typedDims.end()) {
      return m_typedDims.insert(std::make_pair(value, m_nextIndex++)).first->second;
   } else {
      return iter->second;
   }
}

size_t DimensionTranslator::lookup(const ustring_t& name) {
   ustring_size_map::iterator iter = m_namedDims.find(name);
   if (iter == m_namedDims.end()) {
      return 0;
   } else {
      return iter->second;
   }
}

size_t DimensionTranslator::lookup(const TypedValue& value) {
   ustring_type_map::iterator iter = m_typedDims.find(value);
   if (iter == m_typedDims.end()) {
      return 0;
   } else {
      return iter->second;
   }
}

}
