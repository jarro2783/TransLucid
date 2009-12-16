#include "tl/library.hpp"
#include <ltdl.h>
#include <glibmm/ustring.h>
#include <glibmm/convert.h>
#include <iostream>

namespace TransLucid {

namespace {
   bool attemptLibraryOpen(const Glib::ustring& name, TypeRegistry& registry) {
      bool success = true;
      std::string filename = Glib::filename_from_utf8(name);

      lt_dlhandle h = lt_dlopenext(filename.c_str());

      if (h != 0) {
         void* init = lt_dlsym(h, "library_init");

         if (init != 0) {
            (*(library_loader)init)(registry);
         } else {
            success = false;
         }
      } else {
         success = false;
      }

#if 0
      if (!success) {
         std::cerr << "error opening " << name << ":" << std::endl;
         const char * error = lt_dlerror();
         if (error) {
            std::cerr << error << std::endl;
         }
     }
#endif

      return success;
   }
} //namespace anonymous

Libtool::Libtool() {
   lt_dlinit();
   m_searchDirs.push_back(".");
   #ifdef PKGLIBDIR
   m_searchDirs.push_back(PKGLIBDIR);
   #endif
}

Libtool::~Libtool() {
   lt_dlexit();
}

void Libtool::loadLibrary(const Glib::ustring& name, TypeRegistry& registry) {

   bool success = false;

   std::list<ustring_t>::const_iterator iter = m_searchDirs.begin();
   while (!success && iter != m_searchDirs.end()) {
      if (attemptLibraryOpen(*iter + "/lib" + name, registry)) {
         success = true;
      }
      ++iter;
   }

   if (!success) {
      std::cerr << "warning: unable to open library " << name << std::endl;
   }
}

void Libtool::addSearchPath(const ustring_t& path) {
   m_searchDirs.push_back(path);
}

} //namespace TransLucid
