#ifndef LIBRARY_HPP_INCLUDED
#define LIBRARY_HPP_INCLUDED

#include <tl/types.hpp>
#include <ltdl.h>
#include <list>

namespace TransLucid {

   typedef void (*library_loader)(TypeRegistry&);

   class Libtool {
      public:
      Libtool();

      ~Libtool();

      void loadLibrary(const Glib::ustring& name, TypeRegistry& registry);

      void addSearchPath(const ustring_t& path);

      private:
      std::list<ustring_t> m_searchDirs;
   };
} //namespace TransLucid

#endif // LIBRARY_HPP_INCLUDED
