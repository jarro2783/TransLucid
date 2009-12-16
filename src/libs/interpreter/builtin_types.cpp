#include <tl/builtin_types.hpp>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>

using boost::assign::map_list_of;

namespace TransLucid {

Special::StringValueInitialiser Special::m_sv;

Special::StringValueInitialiser::StringValueInitialiser() {
   vtos =
      map_list_of(Special::ERROR, "error")
      (Special::ACCESS, "access")
      (Special::TYPEERROR, "type")
      (Special::DIMENSION, "dim")
      (Special::UNDEF, "undef")
      (Special::CONST, "const")
      (Special::LOOP, "loop");

   BOOST_FOREACH(ValueStringMap::value_type const& v, vtos) {
      stov.insert(std::make_pair(v.second, v.first));
   }
}

}
