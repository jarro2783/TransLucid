#ifndef DIMTRANSLATOR_HPP_INCLUDED
#define DIMTRANSLATOR_HPP_INCLUDED

#include <tl/types.hpp>
#include <boost/unordered_map.hpp>

namespace TransLucid
{
  /**
   * @brief Stores dimensions mapping to integers.
   *
   * Maps typed values and dimensions to integers as a speed
   * optimisation.
   **/
  class DimensionTranslator
  {
    public:

    DimensionTranslator();

    /**
     * @brief Retrieves the value of a named dimension.
     **/
    size_t
    lookup(const u32string& name);

    /**
     * @brief Retrieves the value of a typed value dimension.
     **/
    size_t
    lookup(const TypedValue& value);

    private:

    size_t m_nextIndex;

    typedef boost::unordered_map<u32string, size_t> ustring_size_map;
    typedef boost::unordered_map<TypedValue, size_t> ustring_type_map;

    ustring_size_map m_namedDims;
    ustring_type_map m_typedDims;
  };
}

#endif // DIMTRANSLATOR_HPP_INCLUDED
