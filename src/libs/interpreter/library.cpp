#include "tl/library.hpp"
#include <ltdl.h>
#include <iostream>

namespace TransLucid
{

namespace
{
  bool
  attemptLibraryOpen
  (
    const u32string& file,
    const u32string& name,
    HD* system
  )
  {
    #warning fix library loading
    bool success = true;
    std::string filename;//= Glib::filename_from_utf8(name);

    lt_dlhandle h = lt_dlopenext(filename.c_str());

    if (h != 0)
    {
      //void* init = lt_dlsym(h, (U"lib_" + name + U"_init").c_str());
      void *init = 0;

      if (init != 0)
      {
        (*(library_loader)init)(system);
      }
      else
      {
        success = false;
      }
    }
    else
    {
      success = false;
    }

#if 0
    if (!success)
    {
      std::cerr << "error opening " << name << ":" << std::endl;
      const char* error = lt_dlerror();
      if (error)
      {
        std::cerr << error << std::endl;
      }
    }
#endif

    return success;
  }
} //namespace anonymous

Libtool::Libtool()
{
  lt_dlinit();
  m_searchDirs.push_back(U".");
  #ifdef PKGLIBDIR
  //m_searchDirs.push_back(U PKGLIBDIR);
  #endif
}

Libtool::~Libtool()
{
  lt_dlexit();
}

void
Libtool::loadLibrary(const u32string& name, HD* system)
{

  bool success = false;

  std::list<u32string>::const_iterator iter = m_searchDirs.begin();
  while (!success && iter != m_searchDirs.end())
  {
    if (attemptLibraryOpen(*iter + U"/lib" + name, name, system))
    {
      success = true;
    }
    ++iter;
  }

  if (!success)
  {
    //std::cerr << "warning: unable to open library " << name << std::endl;
  }
}

void
Libtool::addSearchPath(const u32string& path)
{
  m_searchDirs.push_back(path);
}

} //namespace TransLucid
