#include <tl/builtin_types.hpp>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>

using boost::assign::map_list_of;

namespace TransLucid
{

Special::StringValueInitialiser Special::m_sv;

Special::StringValueInitialiser::StringValueInitialiser()
: vtos
{
  {Special::ERROR, U"error"},
  {Special::ACCESS, U"access"},
  {Special::TYPEERROR, U"type"},
  {Special::DIMENSION, U"dim"},
  {Special::UNDEF, U"undef"},
  {Special::CONST, U"const"},
  {Special::LOOP, U"loop"}
}
{
  BOOST_FOREACH(ValueStringMap::value_type const& v, vtos)
  {
    stov.insert(std::make_pair(v.second, v.first));
  }
}

}
